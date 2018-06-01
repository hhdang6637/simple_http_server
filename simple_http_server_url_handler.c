/*
 * simple_http_server_url_handler.c
 *
 *  Created on: May 31, 2018
 *      Author: hhdang
 */

#define _GNU_SOURCE
#include <string.h>
#include <search.h>
#include <malloc.h>
#include "simple_http_server.h"

extern void *rootOfUrlCallbacks;

int path_compare(const void *path1, const void *path2)
{
    return strcmp(((simpple_http_request_handler*) path1)->path, ((simpple_http_request_handler*) path2)->path);
}

int simple_http_server_add_handler(const char *path, urlCallback f)
{
    void *val;

    Dprintf("enter %s\n", __FUNCTION__);

    simpple_http_request_handler* handler = (simpple_http_request_handler*) malloc(
            sizeof(simpple_http_request_handler));

    snprintf(handler->path, PATH_MAX_LENGTH, "%s", path);

    handler->f = f;

    val = tsearch(handler, &rootOfUrlCallbacks, path_compare);
    if (val == NULL) {
        Dprintf("tsearch cannot allocate new node\n");
        return 0;
    } else if ((*(simpple_http_request_handler **) val) != handler) {
        Dprintf("path (%s) was existed in the tree\n", path);
        free(handler);
        return 0;
    }

    Dprintf("tsearch allocate a new node success\n");

    return 1;
}

void simple_http_server_destroy_handlers()
{
    tdestroy(rootOfUrlCallbacks, free);
}
