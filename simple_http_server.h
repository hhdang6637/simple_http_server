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

#define PATH_MAX_LENGTH  		256
#define VERSION_MAX_LENGTH  	64

typedef struct simpple_http_request_header_field
{
    char *name;
    char *data;
} simpple_http_request_header_field;

typedef enum simpple_http_request_method {
    GET         = 0,
    HEAD        = 1,
    POST        = 2,
    PUT         = 3,
    DELETE      = 4,
    CONNECT     = 5,
    OPTIONS     = 6,
    TRACE       = 7,
    PATCH       = 8,
    METHOD_MAX
} simpple_http_request_method;

typedef struct simpple_http_request_line
{
    simpple_http_request_method method;
    char                        path[PATH_MAX_LENGTH];
    char                        version[VERSION_MAX_LENGTH];
} simpple_http_request_line;

#define MAX_FIELDS  20

typedef struct simpple_http_request
{
    simpple_http_request_line           requestLine;
    simpple_http_request_header_field   requestHeaderFields[MAX_FIELDS]; // end with NULL pointer
    int 								numRequestHeaderFields;
} simpple_http_request;

typedef struct simple_http_webs
{
	int 					fd;
	FILE 					*FilePtr;
	simpple_http_request 	*request;
} simple_http_webs;

typedef void (*urlCallback)(simple_http_webs *web);

typedef struct simple_http_request_handler
{
	char path[PATH_MAX_LENGTH];
	urlCallback f;
} simple_http_request_handler;

void simple_http_server_init(int port);
int  simple_http_server_add_handler(const char *path, urlCallback f);
void simple_http_server_destroy_handlers();
void doResponseNotFound(simple_http_webs *web);
void simple_http_server_start();
void simple_http_server_stop();

#endif /* _SIMPLE_HTTP_SERVER_SIMPLE_HTTP_SERVER_H_ */
