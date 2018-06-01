/*
 * util.c
 *
 *  Created on: May 17, 2018
 *      Author: hhdang
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "util.h"

/**
 * return 0 for scuccess
 * return -1 for fail
 */
int fetch_line_from_file(const char*filename, void (*fetch_line_callback)(char *, void *), void *context)
{
    char buf[10 * 1024];
    FILE *fptr;

    fptr = fopen(filename, "r");

    if ((fptr != NULL)) {
        while (fgets(buf, sizeof(buf), fptr) != NULL) {
            fetch_line_callback(buf, context);
        }
        fclose(fptr);
        return 1;
    } else {
        fprintf(stderr, "cannot open file %s\n", filename);
        return 0;
    }
}

char* mapFile2Memory(const char* filename, size_t *file_size) {
    char        *addr;
    int         fd;
    struct stat sb;

    fd = open(filename, O_RDONLY);
    if (fd == -1)
        return NULL;

    if (fstat(fd, &sb) == -1) {
        addr = NULL;
        goto out;
    }

    addr = (char*)mmap(NULL, sb.st_size, PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        addr = NULL;
    }

out:
    close(fd);
    *file_size = sb.st_size;

    return addr;
}

