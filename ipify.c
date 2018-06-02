/*
 * ipify.c
 *
 *  Created on: Jun 2, 2018
 *      Author: hhdang
 */

#include <stdio.h>
#include <string.h>

#include "inet_sockets.h"

#define IPIFY_STR "api.ipify.org"

static void doHttpGet(int fd, char *payload, size_t *payloadlen)
{
	int is_text = 0;
	char buf[1024];
	int len;
	len = snprintf(buf, sizeof(buf),
			"GET / HTTP/1.1\r\n"
			"Host: "IPIFY_STR"\r\n"
			"Connection: close\r\n"
			"\r\n");
	write(fd, buf, len);
	do {
		len = sizeof(buf);
		if (readLine(fd, buf, len) == 0) {
			break;
		}

		if (strcmp(buf, "Content-Type: text/plain\r\n") == 0) {

			is_text = 1;

		} else if (strcmp(buf, "\r\n") == 0) {

			*payloadlen = 0;

			if (is_text) {

				*payloadlen = readLine(fd, buf, len);

//				fprintf(stderr, "payload : %s\n", buf);

				if (*payloadlen) {
					memcpy(payload, buf, *payloadlen);
					payload[*payloadlen] = '\0';
				}
			}

			break;
		}
	} while (1);
}

int get_ip_address_via_ipify(char *ip, size_t *iplen)
{
	int cfd;

	cfd = openTcpClientSocket(IPIFY_STR, 80);
	if (cfd) {
		doHttpGet(cfd, ip, iplen);
		close(cfd);
		return 1;
	}
	return 0;
}

#if 0
int main(int argc, char **argv)
{
	char ip[32];
	size_t len = 32;

	if (get_ip_address_via_ipify(ip, &len)) {
		printf("MyIP : %s\n", ip);
	}

	return 0;
}
#endif
