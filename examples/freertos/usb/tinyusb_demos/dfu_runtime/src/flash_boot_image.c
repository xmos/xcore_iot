// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <string.h>
#include <xcore/assert.h>

#include "rtos_support.h"
#include "flash_boot_image.h"

#define NIBBLE_SWAP_INT(X) (((X&0x0f0f0f0f)<<4) | ((X&0xf0f0f0f0)>>4))

#define SECTORS_TO_BYTES(s, ss) ((s) * (ss))
#define BYTES_TO_SECTORS(b, ss) (((b) + (ss) - 1) / (ss))

#define SECTOR_TO_BYTE_ADDRESS(s, ss) SECTORS_TO_BYTES(s, ss)
#define BYTE_TO_SECTOR_ADDRESS(b, ss) ((b) / (ss))

#define BYTE_IS_SECTOR_BOUNDARY(b, ss) SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(b, ss), ss) == b)

#define IMAGETAG (0x1a551e5)
#define IMAGETAG_13 (0x0FF51DE)

#define IMAGE_PAGE_SIZE 64
#define IMAGE_HEADER_SIZE 7
#define IMAGE_HEADER_SECURE_OFFSET 3

#define IMAGETAG_OFFSET 0
#define IMAGESIZE_OFFSET 3
#define IMAGESIZE_OFFSET_13 4
#define IMAGEVERSION_OFFSET 4
#define IMAGEVERSION_OFFSET_13 5

#define CRC_START_OFFSET_12 4
#define CRC_START_OFFSET_13 3
#define PAGE_CRC_OFFSET 1
#define TOOLS12_EXTRA_SKIP 1

#define S2L_SIZE_OFFSET 0
#define BOOT_PARTITION_OFFSET 12
#define FLASH_ID_ROOT_OFFSET 16

static int boot_image_find_factory_image(boot_image_manager_ctx_t* ctx);
static uint32_t image_crc(uint32_t * data, uint32_t num_words, uint32_t expected_crc);
static int boot_image_locate_next_image(boot_image_manager_ctx_t* ctx);

void boot_image_manager_init(boot_image_manager_ctx_t *ctx, size_t page_size, size_t page_count, void *app_data)
{
    memset(ctx, 0x00, sizeof(boot_image_manager_ctx_t));
    ctx->app_data = app_data;
    ctx->page_size = page_size;
    ctx->page_count = page_count;
}

int boot_image_build_table(boot_image_manager_ctx_t* ctx)
{
    int retval = 0;

    if (boot_image_read(ctx->app_data,
                        BOOT_PARTITION_OFFSET,
                        (uint8_t *)&ctx->boot_partition_size,
                        sizeof(uint32_t))
        != 0)
    {
        retval = boot_image_find_factory_image(ctx);

        if (retval != 0) {
            int next = 0;

            while (retval < MAX_IMAGE_CNT) {
                if ((next = boot_image_locate_next_image(ctx)) == 0) {
                    break;
                } else {
                    retval++;
                }
            }
        }
    }

    return retval;
}

int boot_image_locate_available_spot(boot_image_manager_ctx_t* ctx, uint32_t* addr, size_t* bytes_avail)
{
    boot_image_t* img = &ctx->img_table[ctx->valid_img_cnt - 1];
    if (img != NULL) {
        uint32_t last_image_sector = BYTE_TO_SECTOR_ADDRESS(img->startAddress + img->size, ctx->page_size);
        *addr = SECTOR_TO_BYTE_ADDRESS((last_image_sector + 1), ctx->page_size);
        *bytes_avail = ctx->boot_partition_size - (img->startAddress + img->size);
        return 1;
    }
    return -1;
}

boot_image_t* boot_image_get_last_image(boot_image_manager_ctx_t* ctx)
{
    return &ctx->img_table[MAX_IMAGE_CNT-1];
}

boot_image_t* boot_image_get_factory_image(boot_image_manager_ctx_t* ctx)
{
    return &ctx->img_table[FACTORY_IMAGE_NDX];
}

static int boot_image_find_factory_image(boot_image_manager_ctx_t* ctx)
{
    //Find where the factory image begins
    //The first word in flash represents the size of the S2L and the factory
    //image starts immediately after it.
    uint32_t sizeBuf;

    boot_image_read(ctx->app_data, S2L_SIZE_OFFSET, (uint8_t*)&sizeBuf, sizeof(uint32_t));

    unsigned int startAddr = (NIBBLE_SWAP_INT(sizeBuf) + 2) << 2;

    //It is possible that this is a secure factory image table and therefore
    //There are an extra 3 words at the start of the Factory image that must be
    //accounted for when looking for the image tag.
    unsigned headerBuf[IMAGE_PAGE_SIZE + IMAGE_HEADER_SECURE_OFFSET];

    boot_image_read(ctx->app_data, startAddr, (uint8_t*)&headerBuf, (IMAGE_PAGE_SIZE + IMAGE_HEADER_SECURE_OFFSET) * sizeof(int));

    unsigned * header = headerBuf;
    unsigned int image13 = 0;

    if (NIBBLE_SWAP_INT(header[IMAGETAG_OFFSET]) != IMAGETAG) {
        if (NIBBLE_SWAP_INT(header[IMAGETAG_OFFSET]) != IMAGETAG_13) {
            /* Try secure case. */
            header += IMAGE_HEADER_SECURE_OFFSET;
            if (NIBBLE_SWAP_INT(header[IMAGETAG_OFFSET]) != IMAGETAG) {
                if (NIBBLE_SWAP_INT(header[IMAGETAG_OFFSET]) != IMAGETAG_13) {
                    /* No Factory image has been found */
                    rtos_printf("No factory image found\n");
                    return 0;
                } else {
                    rtos_printf("Secure Image 13 found\n");
                    image13 = 1;
                }
            }
        } else {
            rtos_printf("Image 13 found\n");
            image13 = 1;
        }
    }

    uint32_t page_crc;

    page_crc = image_crc((uint32_t*)(header + CRC_START_OFFSET_13),
                         IMAGE_PAGE_SIZE - CRC_START_OFFSET_13,
                         NIBBLE_SWAP_INT(header[PAGE_CRC_OFFSET]));

    if(page_crc != 0) {
        //Factory image failed CRC
        rtos_printf("Failed CRC\n");
        return 0;
    } else {
        rtos_printf("CRC passed\n");
    }

    /* Image headers changed format in tools 13*/
    ctx->img_table[FACTORY_IMAGE_NDX].startAddress = startAddr;
    ctx->img_table[FACTORY_IMAGE_NDX].size = image13 ? NIBBLE_SWAP_INT(header[IMAGESIZE_OFFSET_13])
                                                     : NIBBLE_SWAP_INT(header[IMAGESIZE_OFFSET]);
    ctx->img_table[FACTORY_IMAGE_NDX].version = image13 ? NIBBLE_SWAP_INT(header[IMAGEVERSION_OFFSET_13])
                                                        : NIBBLE_SWAP_INT(header[IMAGEVERSION_OFFSET]);
    ctx->img_table[FACTORY_IMAGE_NDX].factory = 1;
    ctx->valid_img_cnt++;

    return 1;
}

static uint32_t image_crc(uint32_t *data, uint32_t num_words, uint32_t expected_crc)
{
    uint32_t crc = 0xFFFFFFFF;
    uint32_t polynom = 0xedb88320;

    for(uint32_t i=0; i<num_words; i++)
    {
        asm volatile("crc32 %0, %2, %3" : "=r" (crc) : "0" (crc), "r" (NIBBLE_SWAP_INT(data[i])), "r" (polynom));
    }

    asm volatile("crc32 %0, %2, %3" : "=r" (crc) : "0" (crc), "r" (expected_crc), "r" (polynom));

    return ~crc;
}

// Images may start on a sector boundary
static int boot_image_locate_next_image(boot_image_manager_ctx_t* ctx)
{
    if (ctx->boot_partition_size == 0) {
        return 0;
    }

    boot_image_t *last_image = &ctx->img_table[ctx->valid_img_cnt-1];
    boot_image_t *image = &ctx->img_table[ctx->valid_img_cnt];

    uint32_t last_image_sector = BYTE_TO_SECTOR_ADDRESS(last_image->startAddress + last_image->size, ctx->page_size);
    uint32_t boot_partition_end_sector = BYTE_TO_SECTOR_ADDRESS(ctx->boot_partition_size - 1, ctx->page_size);

    rtos_printf("last boot partition sector is %d\n", boot_partition_end_sector);

    last_image_sector++;    // this now is the first sector that could hold an image

    if (last_image_sector == boot_partition_end_sector) {
        return 0;
    }

    uint32_t headerBuf[IMAGE_PAGE_SIZE];
    uint32_t* header = headerBuf;
    uint32_t addr = SECTOR_TO_BYTE_ADDRESS(last_image_sector, ctx->page_size);

    boot_image_read(ctx->app_data, addr, (uint8_t*)&headerBuf, IMAGE_PAGE_SIZE * sizeof(uint32_t));

    rtos_printf("check sector %d addr 0x%x for image header\n", last_image_sector, addr);

    if (NIBBLE_SWAP_INT(headerBuf[IMAGETAG_OFFSET]) == IMAGETAG_13)
    {
        rtos_printf("tag found @ 0x%x\n", addr);
        uint32_t page_crc = image_crc((uint32_t*)(header + CRC_START_OFFSET_13),
                                      IMAGE_PAGE_SIZE - CRC_START_OFFSET_13,
                                      NIBBLE_SWAP_INT(header[PAGE_CRC_OFFSET]));
        if (page_crc != 0) {
            rtos_printf("crc failed got: 0x%x expected: 0x%x\n", page_crc, NIBBLE_SWAP_INT(header[PAGE_CRC_OFFSET]));
        } else {
            image->startAddress = addr;
            image->size = NIBBLE_SWAP_INT(header[IMAGESIZE_OFFSET_13]);
            image->version = NIBBLE_SWAP_INT(header[IMAGEVERSION_OFFSET_13]);
            image->factory = 0;
            ctx->valid_img_cnt++;
            rtos_printf("image found @ 0x%x, size %u\n", image->startAddress, image->size);
            return 1;
        }
    }

    return 0;
}

__attribute__((weak))
size_t boot_image_read(void* ctx, unsigned addr, uint8_t *buf, size_t len)
{
    (void) ctx;
    (void) addr;
    (void) buf;
    (void) len;

    return 0;
}

__attribute__((weak))
size_t boot_image_write(void* ctx, unsigned addr, const uint8_t *buf, size_t len)
{
    (void) ctx;
    (void) addr;
    (void) buf;
    (void) len;

    return 0;
}
