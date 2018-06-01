/*
 * util.h
 *
 *  Created on: May 17, 2018
 *      Author: hhdang
 */

#ifndef _DATA_STRUCTURES_UTIL_H_
#define _DATA_STRUCTURES_UTIL_H_

int fetch_line_from_file(const char*filename, void (*fetch_line_callback)(char *, void *), void *context);
char* mapFile2Memory(const char* filename, size_t *file_size);

#endif /* _DATA_STRUCTURES_UTIL_H_ */
