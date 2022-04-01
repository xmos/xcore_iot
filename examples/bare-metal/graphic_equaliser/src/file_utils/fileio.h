#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

typedef union {
    int file;
}file_t;

int file_open(FILE *fp, const char* name, const char *mode);
void file_read(FILE *fp, void *buf, size_t count);
void file_write(FILE *fp, void *buf, size_t count);
void file_seek(FILE *fp, long int offset, int origin);
void file_close(FILE *fp);

int get_current_file_offset(FILE *fp);
int get_file_size(FILE *fp);
#endif
