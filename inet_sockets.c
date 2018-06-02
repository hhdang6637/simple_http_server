/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2017.                   *
 *                                                                         *
 * This program is free software. You may use, modify, and redistribute it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 or (at your option)   *
 * any later version. This program is distributed without any warranty.    *
 * See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
 \*************************************************************************/

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

int openTcpServerSocket(int backlog, int port)
{
    struct sockaddr_in6 sockaddr;
    int                 sock;
    int                 rc;

    /*
     *  Create the socket address structure
     */
    memset((char *) &sockaddr, '\0', sizeof(struct sockaddr_in6));
    sockaddr.sin6_family = AF_INET6;
    sockaddr.sin6_port = htons((short) (port & 0xFFFF));

    memset(&sockaddr.sin6_addr.s6_addr[0], 0,
            sizeof(sockaddr.sin6_addr.s6_addr));

    sock = socket(AF_INET6, SOCK_STREAM, 0);

    if (sock < 0) {
        return -1;
    }

    fcntl(sock, F_SETFD, FD_CLOEXEC);

    rc = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &rc, sizeof(rc));
    if (bind(sock, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
        close(sock);
        return -1;
    }

    if (listen(sock, backlog) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

int openTcpClientSocket(const char*hostname, int port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int cfd;

    char port_str[10];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;                /* Allows IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ALL;

    snprintf(port_str, 10, "%d", port);

    if (getaddrinfo(hostname, port_str, &hints, &result) != 0){
        return -1;
    }

    /* Walk through returned list until we find an address structure
     that can be used to successfully connect a socket */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (cfd == -1)
            continue; /* On error, try next address */

        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; /* Success */

        /* Connect failed: close this socket and try next address */

        close(cfd);
    }

    if (rp == NULL) {
        return -1;
    }

    freeaddrinfo(result);

    return cfd;
}

ssize_t readLine(int fd, void *buffer, size_t n)
{
    ssize_t    numRead; /* # of bytes fetched by last read() */
    size_t     totRead; /* Total bytes read so far */
    char     *buf;
    char     ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer; /* No pointer arithmetic on "void *" */

    totRead = 0;
    for (;;) {
        numRead = read(fd, &ch, 1);

        if (numRead == -1) {
            if (errno == EINTR) /* Interrupted --> restart read() */
                continue;
            else
                return -1; /* Some other error */

        } else if (numRead == 0) { /* EOF */
            if (totRead == 0) /* No bytes read; return 0 */
                return 0;
            else
                /* Some bytes read; add '\0' */
                break;

        } else { /* 'numRead' must be 1 if we get here */
            if (totRead < n - 1) { /* Discard > (n - 1) bytes */
                totRead++;
                *buf++ = ch;
            }

            if (ch == '\n')
                break;
        }
    }

    *buf = '\0';
    return totRead;
}
