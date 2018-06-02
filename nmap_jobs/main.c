/*
 * main.c
 *
 *  Created on: Jun 2, 2018
 *      Author: hhdang
 */

#include <stdio.h>
#include <string.h>

#include "ipify.h"

typedef struct nmap_host {
    char    ip[32];
    int     open_ports[16];
} nmap_host;

static void dump_nmap_host (nmap_host *info) {
    int *p = info->open_ports;

    fprintf(stderr, "ip: %15s -> ", info->ip);

    while(*p) {
        fprintf(stderr, "%d ", *p);
        p++;
    }

    fprintf(stderr, "\n");
}

static int parse_line_2_nmap_host(const char *line, nmap_host *host_info) {
    const char *next;
    const char *pre;
    char tmp[512];
    char tmp2[512];
    int  offset;

    int port;
    int *p;

    memset(host_info, 0, sizeof(nmap_host));

    static const char* port_key_word = "Ports: ";

    p = host_info->open_ports;

    pre = line;
    next = strstr(pre, port_key_word);

    if (next) {
        // we found something like below
        // Host: 115.72.54.227 ()   Ports: 21/filtered/tcp//ftp///, 22/closed/tcp//ssh///, 80/closed/tcp//http///, 443/closed/tcp//https///
        // we will parse it to struct nmap_host

        offset = next - pre;
        memcpy(tmp, pre, offset);
        tmp[offset] = '\0';

        if (sscanf (tmp, "Host: %s ()", host_info->ip) == 1) {

            pre = next + strlen(port_key_word);

            while((next = strstr(pre, ", "))) {

                offset = next - pre;
                memcpy(tmp, pre, offset);
                tmp[offset] = '\0';

                if (sscanf (tmp, "%d/%s", &port, tmp2) == 2 && strstr(tmp2, "open")) {
                    *p = port;
                }

                pre = next + 2;
            }

            if (host_info->open_ports[0]) {
                dump_nmap_host(host_info);
                return 1;
            }
        }
    }
    return 0;
}

static void run_nmap(const char *ip) {
    FILE *f;
    char command[128];
    char port_list[128];
    char subnet[128];
    char buffer[1024];
    nmap_host host_info;

    snprintf(port_list, sizeof(port_list), "80,443,22,21");
    snprintf(subnet, sizeof(subnet), "%s/16", ip);

    static const char* NMAP_COMMAND = "nmap -n -oG - ";

    snprintf(command, sizeof(command), "%s -p %s %s", NMAP_COMMAND, port_list, subnet);
    fprintf(stderr, "command: %s\n", command);

    f = popen(command, "r");
    if (f) {
        while (fgets(buffer, sizeof(buffer), f) != NULL) {
            parse_line_2_nmap_host(buffer, &host_info);
        }
        fclose(f);
    }
}

int main(int argc, char **argv) {
    char ip[32];
    size_t len = 32;

    if (get_ip_address_via_ipify(ip, &len)) {

        printf("Your IP : %s\n", ip);
        run_nmap(ip);

    } else {
        fprintf(stderr, "cannot get your ip from ipify\n");
    }

    return 0;
}

