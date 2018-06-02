/*
 * simple_http_server.c
 *
 *  Created on: May 29, 2018
 *      Author: hhdang
 */

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <malloc.h>
#include <string.h>
#include <search.h>

#include "inet_sockets.h"
#include "simple_http_server.h"

/*****************************************************************************/

static int simpple_http_request_port = 0;
static int is_running = 0;
void *rootOfUrlCallbacks;

/*****************************************************************************/

static void destroy_simpple_http_request(simpple_http_request *request)
{
    int i;

    for (i = 0; i < request->numRequestHeaderFields; ++i) {
        if (request->requestHeaderFields[i].name) {
            free(request->requestHeaderFields[i].name);
        }
        if (request->requestHeaderFields[i].data) {
            free(request->requestHeaderFields[i].data);
        }
    }

    free(request);
}

static simpple_http_request_method simpple_http_request_get_method(const char *method_str)
{
    if (strncmp("GET", method_str, sizeof("GET")) == 0) {
        return GET;

    } else if (strncmp("HEAD", method_str, sizeof("HEAD")) == 0) {
        return HEAD;

    } else if (strncmp("POST", method_str, sizeof("POST")) == 0) {
        return POST;

    } else if (strncmp("PUT", method_str, sizeof("PUT")) == 0) {
        return PUT;

    } else if (strncmp("DELETE", method_str, sizeof("DELETE")) == 0) {
        return DELETE;

    } else if (strncmp("CONNECT", method_str, sizeof("CONNECT")) == 0) {
        return CONNECT;

    } else if (strncmp("OPTIONS", method_str, sizeof("OPTIONS")) == 0) {
        return OPTIONS;

    } else if (strncmp("TRACE", method_str, sizeof("TRACE")) == 0) {
        return TRACE;

    } else if (strncmp("PATCH", method_str, sizeof("PATCH")) == 0) {
        return PATCH;

    }

    return METHOD_MAX;
}

static int parse_simpple_http_request_first_line(const char *line, simpple_http_request *header)
{
    const char  *tmpstr;
    const char  *curstr;
    char        method_str[64];
    int         len;

    Dprintf("%s with line : %s\n", __FUNCTION__,line);

    curstr = line;
    tmpstr = strchr(curstr, ' ');
    if (NULL == tmpstr) {
        return 0;
    }

    len = tmpstr - curstr;
    memcpy(method_str, curstr, len);
    method_str[len] = '\0';

    Dprintf("found method : %s\n", method_str);
    header->requestLine.method = simpple_http_request_get_method(method_str);

    /* Skip a space */
    tmpstr++;
    curstr = tmpstr;

    tmpstr = strchr(curstr, ' ');
    if (NULL == tmpstr) {
        return 0;
    }

    len = tmpstr - curstr;
    memcpy(header->requestLine.path, curstr, len);
    header->requestLine.path[len] = '\0';
    Dprintf("found path : %s\n", header->requestLine.path);

    /* Skip a space */
    tmpstr++;

    len = strlen(tmpstr);
    strncpy(header->requestLine.version, tmpstr, sizeof(header->requestLine.version));
    header->requestLine.version[sizeof(header->requestLine.version) - 1] = '\0';
    Dprintf("found http version : %s\n", header->requestLine.version);

    return 1;
}

static int parse_simpple_http_request_next_line(const char *line, simpple_http_request *header)
{
    const char *tmpstr;
    const char *curstr;
    int len;

    Dprintf("%s with line : %s\n", __FUNCTION__,line);

    curstr = line;

    tmpstr = strchr(curstr, ':');
    if (NULL != tmpstr) {
        len = tmpstr - curstr;

        header->requestHeaderFields[header->numRequestHeaderFields].name = (char*) malloc(sizeof(char) * (len + 1));
        strncpy(header->requestHeaderFields[header->numRequestHeaderFields].name, curstr, len);
        header->requestHeaderFields[header->numRequestHeaderFields].name[len] = '\0';

        Dprintf("found data name : %s\n", header->requestHeaderFields[header->numRequestHeaderFields].name);

        /* Skip ':' and space */
        tmpstr ++;

        if (tmpstr[0] == ' ') {
            tmpstr++;
        }

        len = strlen(tmpstr);
        header->requestHeaderFields[header->numRequestHeaderFields].data = (char*) malloc(sizeof(char) * (len + 1));
        strncpy(header->requestHeaderFields[header->numRequestHeaderFields].data, tmpstr, len);
        header->requestHeaderFields[header->numRequestHeaderFields].data[len] = '\0';

        Dprintf("found data value : %s\n", header->requestHeaderFields[header->numRequestHeaderFields].data);

        header->numRequestHeaderFields++;

        return 1;
    }

    Dprintf("something wrong with line : %s\n", line);
    return 0;
}

static simpple_http_request* parse_simpple_http_request(simple_http_webs *webs)
{
    int first = 1;
    char buf[1024];

    simpple_http_request *header = (simpple_http_request *) malloc(sizeof(simpple_http_request));
    memset(header, 0, sizeof(simpple_http_request));

    do {

        if (fgets(buf, sizeof(buf), webs->FilePtr) == NULL) {
            break;
        }

        if (first) {

            if (parse_simpple_http_request_first_line(buf, header) == 0) {
                break;
            }

            first = 0;

        } else {

            if (strcmp(buf, "\r\n") == 0 || header->numRequestHeaderFields == MAX_FIELDS) {
                break;
            } else {
                if (parse_simpple_http_request_next_line(buf, header) == 0) {
                    break;
                }
            }
        }
    } while (1);

    if (first) {
        free(header);
        header = NULL;
    }

    webs->request = header;

    return header;
}
static simple_http_webs* create_simple_http_webs(int fd);
static void doResponse(simple_http_webs *webs);
static void handleRequest(int cfd);

extern int path_compare(const void *path1, const void *path2);

static simple_http_webs* create_simple_http_webs(int fd)
{
    simple_http_webs* webs = (simple_http_webs*) malloc(
            sizeof(simple_http_webs));
    if (webs) {
        memset(webs, 0, sizeof(simple_http_webs));
        webs->fd = fd;
        webs->FilePtr = fdopen(webs->fd, "r+b");
    }

    return webs;
}

static void destroy_simple_http_webs(simple_http_webs*webs)
{
    if (webs) {

        fclose(webs->FilePtr);
        webs->FilePtr = NULL;
        webs->fd = 0;

        if (webs->request) {
            destroy_simpple_http_request(webs->request);
            webs->request = NULL;
        }

        free(webs);
    }
}


static void doResponse(simple_http_webs *webs)
{
    simple_http_request_handler h;
    void *found;
    memcpy(h.path, webs->request->requestLine.path, PATH_MAX_LENGTH);

    if ((found = tfind((const void*) &h, &rootOfUrlCallbacks, path_compare)) != NULL) {
        (*((simple_http_request_handler**) found))->f(webs);
    } else {
        Dprintf("cannot handle url : %s\n", webs->request->requestLine.path);
        doResponseNotFound(webs);
    }
}

static void handleRequest(int cfd)
{
    simple_http_webs *webs = create_simple_http_webs(cfd);
    if (webs) {

        parse_simpple_http_request(webs);

        if (webs->request) {
            Dprintf("request line : %d %s %s\n", webs->request->requestLine.method,
                    webs->request->requestLine.path, webs->request->requestLine.version);

            int i;
            for (i = 0; i < webs->request->numRequestHeaderFields; ++i) {

                Dprintf("HeaderFields #%d : %s : %s", i,
                        webs->request->requestHeaderFields[i].name,
                        webs->request->requestHeaderFields[i].data);
            }
            doResponse(webs);

        } else {
            Dprintf("parse_simpple_http_request fail\n");
        }

        destroy_simple_http_webs(webs);
    }
}

#if 0
static void doResponseHtml(FILE *f, const char *html_content)
{
    fprintf(f, "HTTP/1.1 200"
            "\r\n"
    "content-length: %lu"
            "\r\n"
    "content-type: text/html; charset=iso-8859-1"
            "\r\n"
            "\r\n"
            "%s"
    , strlen(html_content), html_content);
}
#endif

void doResponseNotFound(simple_http_webs *web)
{
    fprintf(web->FilePtr, "HTTP/1.1 404"
            "\r\n"
            "\r\n");
}

void simple_http_server_init(int port) {
    simpple_http_request_port = port;
}

void simple_http_server_start()
{
    int sfd;
    int cfd;

    struct sockaddr_in6 addr;
    socklen_t len;

    Dprintf("start simple http server on port %d\n", simpple_http_request_port);

    if (simpple_http_request_port == 0) {
        fprintf(stderr, "listen port is %d, we maynot call simple_http_server_init\n", simpple_http_request_port);
        return;
    }

    sfd = openTcpServerSocket(10, simpple_http_request_port);

    if (!sfd) {
        return;
    }

    is_running = 1;

    for (; is_running != 0;) {

        Dprintf("waiting for new connection\n");

        len = sizeof(struct sockaddr_in6);
        cfd = accept(sfd, (struct sockaddr *) &addr, &len); /* Wait for connection */
        if (cfd == -1) {

            Dprintf("accept fail errno = %d\n", errno);

            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        }

        handleRequest(cfd);
    }

    Dprintf("exit loop\n");
}

void simple_http_server_stop()
{
    Dprintf("Stoping\n");
    is_running = 0;
}
