// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <ff.h>
#include "fatfs_ops.h"
#include "ramdisk.h"

static FATFS fs;

static char *fatfs_error_str[] = {
        [FR_OK] =                  "Succeeded",
        [FR_DISK_ERR] =            "A hard error occurred in the low level disk I/O layer",
        [FR_INT_ERR] =             "Assertion failed",
        [FR_NOT_READY] =           "The physical drive cannot work",
        [FR_NO_FILE] =             "Could not find the file",
        [FR_NO_PATH] =             "Could not find the path",
        [FR_INVALID_NAME] =        "The path name format is invalid",
        [FR_DENIED] =              "Access denied due to prohibited access or directory full",
        [FR_EXIST] =               "Access denied due to prohibited access",
        [FR_INVALID_OBJECT] =      "The file/directory object is invalid",
        [FR_WRITE_PROTECTED] =     "The physical drive is write protected",
        [FR_INVALID_DRIVE] =       "The logical drive number is invalid",
        [FR_NOT_ENABLED] =         "The volume has no work area",
        [FR_NO_FILESYSTEM] =       "There is no valid FAT volume",
        [FR_MKFS_ABORTED] =        "The f_mkfs() aborted due to any problem",
        [FR_TIMEOUT] =             "Could not get a grant to access the volume within defined period",
        [FR_LOCKED] =              "The operation is rejected according to the file sharing policy",
        [FR_NOT_ENOUGH_CORE] =     "LFN working buffer could not be allocated",
        [FR_TOO_MANY_OPEN_FILES] = "Number of open files > FF_FS_LOCK",
        [FR_INVALID_PARAMETER] =   "Given parameter is invalid",
};

void *fatfs_init(size_t *image_size, size_t cluster_size)
{
    BYTE mkfs_work[8*FF_MAX_SS];

    const MKFS_PARM mkfs_params = {
            .fmt = FM_FAT | FM_FAT32 | FM_SFD,
            .n_fat = 1,
            .align = 0, /* default */
            .n_root = 0, /* default */
            .au_size = cluster_size
    };

    FRESULT res;
    res = f_mkfs("", &mkfs_params, mkfs_work, sizeof(mkfs_work));
    if (res != FR_OK) {
        fprintf(stderr, "Failed to create FAT volume: %s.\nAdjust volume size or cluster size.\n", fatfs_error_str[res]);
        return NULL;
    }

    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        fprintf(stderr, "Failed to mount FAT volume: %s.\n", fatfs_error_str[res]);
        return NULL;
    }

    return RAM_disk_raw(image_size);
}

int fatfs_dir_enter(char *name)
{
    FRESULT res;

    if (((res = f_mkdir(name)) == FR_OK) &&
        ((res = f_chdir(name)) == FR_OK)) {
        return 0;
    } else {
        fprintf(stderr, "Failed to create and enter directory %s: %s.\n", name, fatfs_error_str[res]);
        return -1;
    }
}

int fatfs_dir_up(void)
{
    FRESULT res;

    if ((res = f_chdir("..")) == FR_OK) {
        return 0;
    } else {
        fprintf(stderr, "Failed to change directory: %s.\n", fatfs_error_str[res]);
        return -1;
    }
}

int fatfs_file_copy(char *name, FILE *fp)
{
    BYTE data[1024];
    size_t bytes_read;
    FIL fil;
    FRESULT res;
    int ret = 0;

    if ((res = f_open(&fil, name, FA_CREATE_ALWAYS | FA_WRITE)) == FR_OK) {

        while ((bytes_read = fread(data, 1, sizeof(data), fp)) > 0) {
            UINT bw;
            if ((res = f_write(&fil, data, bytes_read, &bw)) != FR_OK) {
                fprintf(stderr, "Failed to write data to file %s: %s.\n", name, fatfs_error_str[res]);
                ret = -1;
                break;
            } else if (bytes_read != bw) {
                fprintf(stderr, "Failed to write all data to file %s. Image size too small?\n", name);
                ret = -1;
                break;
            }
        }

        if ((res = f_close(&fil)) != FR_OK) {
            fprintf(stderr, "Failed to close file %s: %s.\n", name, fatfs_error_str[res]);
            ret = -1;
        }
    } else {
        fprintf(stderr, "Failed to create file %s: %s.\n", name, fatfs_error_str[res]);
        ret = -1;
    }

    return ret;
}
