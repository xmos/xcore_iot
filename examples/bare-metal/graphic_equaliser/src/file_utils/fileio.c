
#include "fileio.h"

int file_open(FILE *fp, const char* name, const char *mode) {

    if(!strcmp(mode, "rb")||!strcmp(mode, "wb")) {
        fp = fopen(name, mode);
        if(fp == NULL) {return -1;}
    }
    else {
        assert((0) && "invalid file open mode specified. Only 'rb' and 'wb' modes supported");
    }
    return 0;
}

void file_seek(FILE *fp, long int offset, int whence) {

    fseek(fp, offset, whence);

}

int get_current_file_offset(FILE *fp) {

    return ftell(fp);

}

int get_file_size(FILE *fp) {

    //find the current offset in the file
    int current_offset = ftell(fp);
    //get file size
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    //go back to original offset
    fseek(fp, current_offset, SEEK_SET);

    return size;
}

void file_read(FILE *fp, void *buf, size_t count) {

    fread(buf, count, 1, fp);

}

void file_write(FILE *fp, void *buf, size_t count) {

    fwrite(buf, count, 1, fp);

}

void file_close(FILE *fp){
    fclose(fp);
}
