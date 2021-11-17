// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <stdio.h>
#include <string.h>
#include "tinydir.h"
#include "fatfs_ops.h"

static int dir_recurse(tinydir_dir dir)
{
    int ret = 0;

    while (dir.has_next) {
        tinydir_file file;
        if (tinydir_readfile(&dir, &file) != 0) {
            return -1;
        }

        if (file.is_dir) {
            if (strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0) {
                tinydir_dir subdir;
                if (tinydir_open(&subdir, file.path) != 0 ||
                    fatfs_dir_enter(file.name) != 0       ||
                    dir_recurse(subdir) != 0              ||
                    fatfs_dir_up()) {
                    ret = -1;
                    break;
                }
            }

        } else if (file.is_reg) {
            FILE *fp = fopen(file.path, "rb");
            if (fp == NULL) {
                ret = -1;
            }

            if (ret == 0) {
                ret = fatfs_file_copy(file.name, fp);
                fclose(fp);
            }

            if (ret != 0) {
                break;
            }
        }

        tinydir_next(&dir);
    }

    return ret;
}

int directory_add(const char *dirname)
{
    int ret;
    tinydir_dir dir;

    if ((ret = tinydir_open(&dir, dirname)) == 0) {
        ret = dir_recurse(dir);
    } else {
        fprintf(stderr, "Error opening specified directory %s\n", dirname);
    }

    return ret;
}
