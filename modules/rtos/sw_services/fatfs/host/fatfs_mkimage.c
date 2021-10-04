// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <stdio.h>
#include <ff.h>
#include "argtable/argtable3.h"
#include "fatfs_ops.h"
#include "directory_add.h"

#define VERSION "1.0-RC1"

#define IMAGE_SIZE_DEFAULT          1024*1024
#define SECTOR_SIZE_DEFAULT         4096
#define SECTORS_PER_CLUSTER_DEFAULT 1

size_t image_size_g;
size_t image_sector_size_g;

int main(int argc, char **argv)
{
    int ret = 0;
    void *image;
    size_t image_size_actual;

    struct arg_lit *help, *version;
    struct arg_int *image_size, *sector_size, *sectors_per_cluster;
    struct arg_file *output_file, *input_dir;
    struct arg_end *end;

    void *argtable[] = {
        help    = arg_lit0("h", "help", "Display this help and exit."),
        version = arg_lit0(NULL, "version", "Display version info and exit."),

        output_file = arg_file1("o", "output", "<file>", "The name of the output file system image file."),
        input_dir   = arg_file1("i", "input", "<directory>", "The name of the input directory to create the file system from."),

        image_size = arg_int0("s", "image_size", "<n>", "The size in bytes of the file system image. Default is 1048576."),
        sector_size = arg_int0("S", "sector_size", "<n>", "The size in bytes of the file system's sectors. Default is 4096."),
        sectors_per_cluster = arg_int0("c", "sectors_per_cluster", "<n>", "The number of sectors per cluster. Must be a power of 2. Default is 1."),

        end = arg_end(8),
    };

    int nerrors;
    nerrors = arg_parse(argc, argv, argtable);

    if (help->count > 0) {
        printf("Usage: %s", argv[0]);
        arg_print_syntax(stdout, argtable, "\n\n");
        printf("FAT file system image creation tool\n\n");
        printf("This tool creates a FAT filesystem image that is populated with the contents\n");
        printf("of the specified directory. All filenames must be in 8.3 format.\n\n");
        arg_print_glossary_gnu(stdout, argtable);
        return 0;
    }

    if (version->count > 0) {
        printf("fatfs_mkimage verion %s\n", VERSION);
        return 0;
    }

    if (nerrors > 0) {
        arg_print_errors(stdout, end, argv[0]);
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return 1;
    }

    if (image_size->count > 0) {
        if (image_size->ival[0] > 0) {
            image_size_g = image_size->ival[0];
        } else {
            fprintf(stderr, "Image size must be greater than zero\n");
            return 1;
        }
    } else {
        image_size_g = IMAGE_SIZE_DEFAULT;
    }

    if (sector_size->count > 0) {
        if (sector_size->ival[0] > 0) {
            image_sector_size_g = sector_size->ival[0];
        } else {
            fprintf(stderr, "Sector size must be greater than zero\n");
            return 1;
        }
    } else {
        image_sector_size_g = SECTOR_SIZE_DEFAULT;
    }

    if (sectors_per_cluster->count == 0) {
        sectors_per_cluster->count = 1;
        sectors_per_cluster->ival[0] = SECTORS_PER_CLUSTER_DEFAULT;
    }

    image = fatfs_init(&image_size_actual, sectors_per_cluster->ival[0] * image_sector_size_g);

    if (image != NULL && directory_add(input_dir->filename[0]) == 0) {
        FILE *outfile;
        outfile = fopen(output_file->filename[0], "wb");
        if (outfile != NULL) {
            if (fwrite(image, sizeof(BYTE), image_size_actual, outfile) != image_size_actual) {
                fprintf(stderr, "Failed to write filesystem to file %s\n", output_file->filename[0]);
                ret = 1;
            }
            fclose(outfile);
        } else {
            fprintf(stderr, "Failed to open file %s for writing\n", output_file->filename[0]);
            ret = 1;
        }
    } else {
        ret = 1;
    }

    if (ret == 0) {
        printf("Filesystem image %s successfully populated with contents of directory %s\n", output_file->filename[0], input_dir->filename[0]);
    } else {
        fprintf(stderr, "Failed to create filesystem image %s\n", output_file->filename[0]);
    }

    return ret;
}
