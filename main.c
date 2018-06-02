/*
 * main.c
 *
 *  Created on: May 31, 2018
 *      Author: hhdang
 */

#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>

#include "simple_http_server.h"
#include "util.h"

static void siginfoHandler(int sig, siginfo_t *si, void *ucontext)
{
    simple_http_server_stop();
}

static void register_signal_handler()
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = siginfoHandler;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &act, NULL) == -1){
        fprintf(stderr, "Cannot register signal handlr for SIGINT\n");
    }
}

static void urlIndexHandler(simple_http_webs *webs)
{
    const char*str = "<script>window.location = '/index.html'</script>";

    fprintf(webs->FilePtr, "HTTP/1.1 200"
            "\r\n"
            "content-length: %zu"
            "\r\n"
            "content-type: text/html; charset=iso-8859-1"
            "\r\n"
            "\r\n"
            "%s", strlen(str), str);
}

static void showFile(simple_http_webs *webs)
{
    int fd;
    char filename[300];
    char buf[512];
    int len;
    struct stat sta_info;

    snprintf(filename, sizeof(filename), "html/%s",
            webs->request->requestLine.path);

    fd = open(filename, O_RDONLY);
    if (fd && (fstat(fd, &sta_info) == 0)) {

        len = snprintf(buf, sizeof(buf), "HTTP/1.1 200"
                "\r\n"
                "content-length: %zu"
                "\r\n"
                "content-type: text/html; charset=iso-8859-1"
                "\r\n"
                "\r\n", sta_info.st_size);

        write(webs->fd, buf, len);

        sendfile(webs->fd, fd, 0, sta_info.st_size);

    } else {
        doResponseNotFound(webs);
    }

    if (fd != -1) {
        close(fd);
    }
}

int main(int argc, char **argv)
{
    register_signal_handler();

    simple_http_server_init(6637);

    simple_http_server_add_handler("/", urlIndexHandler);
    simple_http_server_add_handler("/index.html", showFile);

    simple_http_server_start();

    simple_http_server_destroy_handlers();

    return 0;
}
