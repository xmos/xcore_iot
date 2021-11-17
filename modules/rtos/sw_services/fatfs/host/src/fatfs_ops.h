// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef FATFS_OPS_H_
#define FATFS_OPS_H_

#include <stdio.h>

void *fatfs_init(size_t *image_size, size_t cluster_size);
int fatfs_dir_enter(char *name);
int fatfs_dir_up(void);
int fatfs_file_copy(char *name, FILE *fp);


#endif /* FATFS_OPS_H_ */
