// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef FLASH_BOOT_IMAGE_H_
#define FLASH_BOOT_IMAGE_H_

#include <stdint.h>

#ifndef MAX_IMAGE_CNT
#define MAX_IMAGE_CNT       2
#endif

#define FACTORY_IMAGE_NDX   0

/**
 * \defgroup flash_boot_image_manager
 *
 * The public API for using the flash boot image manager.
 * @{
 */

/** Struct describing a bootable image. */
typedef struct {
    uint32_t startAddress;    /**< The address of the start of the image. */
    uint32_t size;            /**< The size in bytes of the image. */
    uint32_t version;         /**< The image version. */
    uint32_t factory;         /**< 1 if the image is the factory image, 0 otherwise. */
} boot_image_t;

/**
 * Typedef to the flash boot image manager instance struct.
 */
typedef struct boot_image_manager_struct boot_image_manager_ctx_t;

/**
 * Struct representing an flash boot image manager instance.
 *
 */
struct boot_image_manager_struct {
    /**
     * A pointer to application specific data.  Used to share data with callback
     * functions and the application implementation.
     */
    void* app_data;

    /**
     * A pointer to the image table
     */
    boot_image_t img_table[MAX_IMAGE_CNT];

	/**
	 * The number of valid images populated
	 */
    size_t valid_img_cnt;

	/**
	 * The page size in bytes of the underlying flash device
	 */
    size_t page_size;

 	/**
 	 * The page count of the underlying flash device
 	 */
    size_t page_count;

 	/**
 	 * The size of the boot partition in bytes
 	 */
    uint32_t boot_partition_size;
};

unsigned int boot_image_validate(boot_image_t *image);

/**
 * User defined read function
 *
 * \param ctx   A pointer to a user context
 * \param addr  The address to read from
 * \param buf   A pointer to the buffer to read into
 * \param len   The number of bytes to read
 *
 * \return number of bytes read
 */
size_t boot_image_read(void* ctx, unsigned addr, uint8_t *buf, size_t len);

/**
 * User defined write function
 *
 * \param ctx   A pointer to a user context
 * \param addr  The address to read from
 * \param buf   A pointer to the buffer to write from
 * \param len   The number of bytes to write
 *
 * \return number of bytes written
 */
size_t boot_image_write(void* ctx, unsigned addr, const uint8_t *buf, size_t len);

/**
 * Populates the boot image table.

 * \param ctx       A pointer to the boot image manager context
 *
 * \return number of images found
 */
int boot_image_build_table(boot_image_manager_ctx_t *ctx);

/**
 * Populates the boot image table.

 * \param ctx         A pointer to the boot image manager context
 * \param addr        A pointer to populate with a found address
 * \param bytes_avail A pointer to populate with a number of bytes available to
 *                    the end of the boot parition from addr
 *
 * \return 1 if a spot was located
 *         0 otherwise
 */
int boot_image_locate_available_spot(boot_image_manager_ctx_t* ctx, uint32_t* addr, size_t* bytes_avail);

/**
 * Get the factory image struct

 * \param ctx         A pointer to the boot image manager context
 *
 * \return Pointer to the factory image struct
 */
boot_image_t* boot_image_get_factory_image(boot_image_manager_ctx_t* ctx);

/**
 * Get the last populated image struct

 * \param ctx         A pointer to the boot image manager context
 *
 * \return Pointer to the last populated image struct
 */
boot_image_t* boot_image_get_last_image(boot_image_manager_ctx_t* ctx);

/**
 * Initializes the boot_image_manager_ctx
 *
 * \param ctx        A pointer to the boot image manager context
 * \param page_size  The size in bytes of the underlying flash page
 * \param page_count The number of pages in the underlying flash
 * \param app_data   A pointer to the application specific data
 */
void boot_image_manager_init(boot_image_manager_ctx_t *ctx, size_t page_size, size_t page_count, void *app_data);

/**@}*/

#endif /* FLASH_BOOT_IMAGE_H_ */
