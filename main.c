/*
 * main.c
 *
 *  Created on: May 31, 2018
 *      Author: hhdang
 */

#include <signal.h>
#include <string.h>

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
    const char*str = "<H1>HELLO!</H1>";

    fprintf(webs->FilePtr, "HTTP/1.1 200"
            "\r\n"
            "content-length: %zu"
            "\r\n"
            "content-type: text/html; charset=iso-8859-1"
            "\r\n"
            "\r\n"
            "%s", strlen(str), str);
}

int main(int argc, char **argv)
{
    register_signal_handler();

    simple_http_server_init(6637);

    simple_http_server_add_handler("/", urlIndexHandler);

    simple_http_server_start();

    simple_http_server_destroy_handlers();

    return 0;
}
