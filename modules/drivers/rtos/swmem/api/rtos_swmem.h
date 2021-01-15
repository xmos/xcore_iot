// Copyright (c) 2021, XMOS Ltd, All rights reserved

#ifndef RTOS_SWMEM_H_
#define RTOS_SWMEM_H_

#include "drivers/rtos/osal/api/rtos_osal.h"

/**
 * Flag indicating that software memory reads should be enabled.
 * This should probably always be set when using software memory.
 */
#define RTOS_SWMEM_READ_FLAG  0x01

/**
 * Flag indicating that software memory writes should be enabled.
 * This will not always need to be set, especially if flash is
 * backing the software memory and intended to be read only.
 */
#define RTOS_SWMEM_WRITE_FLAG 0x02

/**
 * Services a software memory read request. This function should be provided
 * by the application when the software memory driver was initialized with
 * the RTOS_SWMEM_READ_FLAG flag.
 *
 * \param offset The byte offset into the software memory of the cache line
 *               that has had a cache miss.
 * \param buf    This function must fill this with SWMEM_EVICT_SIZE_WORDS
 *               words of data. Where this data comes from is up to the
 *               application. One example is from a flash memory.
 */
void rtos_swmem_read_request(unsigned offset, uint32_t *buf);

/**
 * Services a software memory write request. This function should be provided
 * by the application when the software memory driver was initialized with
 * the RTOS_SWMEM_WRITE_FLAG flag.
 *
 * \param offset     The byte offset into the software memory of the cache line
 *                   that is being evicted.
 * \param dirty_mask A bytewise dirty mask for the data in \p buf. The least
 *                   significant bit corresponds to the lowest byte address in
 *                   \p buf and each subsequent byte address corresponds to the
 *                   next least significant bit.
 * \param buf        A pointer to a buffer containing SWMEM_EVICT_SIZE_WORDS
 *                   words of data from the cache line being evicted. It is up
 *                   to the application what it does with this data. One example
 *                   is to write it to flash memory.
 */
void rtos_swmem_write_request(unsigned offset, uint32_t dirty_mask, const uint32_t *buf);

/**
 * Starts the RTOS software memory driver.
 *
 * \param priority  The priority of the task that gets created by the driver to
 *                  service the software memory.
 */
void rtos_swmem_start(unsigned priority);

/**
 * Initializes the software memory for use by the RTOS software memory driver.
 *
 * \param init_flags A bitfield consisting of initialization flags.
 *                   - RTOS_SWMEM_READ_FLAG enables swmem reads.
 *                   - RTOS_SWMEM_WRITE_FLAG enables swmem writes.
 */
void rtos_swmem_init(uint32_t init_flags);

#endif /* RTOS_SWMEM_H_ */
