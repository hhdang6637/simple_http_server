/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2017.                   *
 *                                                                         *
 * This program is free software. You may use, modify, and redistribute it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 or (at your option)   *
 * any later version. This program is distributed without any warranty.    *
 * See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
 \*************************************************************************/

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>

int openTcpServerSocket(int backlog, int port)
{
    struct sockaddr_in6 sockaddr;
    int sock;
    int rc;

    /*
     *  Create the socket address structure
     */
    memset((char *) &sockaddr, '\0', sizeof(struct sockaddr_in6));
    sockaddr.sin6_family = AF_INET6;
    sockaddr.sin6_port = htons((short) (port & 0xFFFF));

    memset(&sockaddr.sin6_addr.s6_addr[0], 0, sizeof(sockaddr.sin6_addr.s6_addr));

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
