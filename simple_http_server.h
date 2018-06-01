/*
 * simple_http_server.h
 *
 *  Created on: May 31, 2018
 *      Author: hhdang
 */

#ifndef _SIMPLE_HTTP_SERVER_SIMPLE_HTTP_SERVER_H_
#define _SIMPLE_HTTP_SERVER_SIMPLE_HTTP_SERVER_H_

#include <stdio.h>

#define Dprintf(format, ...) fprintf (stderr,"%s:%d : " format , __FILE__, __LINE__, ##__VA_ARGS__)

#define PATH_MAX_LENGTH  256
#define VERSION_MAX_LENGTH  64

typedef void (*urlCallback)(FILE *);

typedef struct simpple_http_request_handler
{
    char path[PATH_MAX_LENGTH];
    urlCallback f;
} simpple_http_request_handler;

void simple_http_server_init(int port);
int  simple_http_server_add_handler(const char *path, urlCallback f);
void simple_http_server_destroy_handlers();
void doResponseNotFound(FILE *f);
void simple_http_server_start();
void simple_http_server_stop();

#endif /* _SIMPLE_HTTP_SERVER_SIMPLE_HTTP_SERVER_H_ */
