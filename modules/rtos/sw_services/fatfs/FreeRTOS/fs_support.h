// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef FS_SUPPORT_H_
#define FS_SUPPORT_H_

#include "ff.h"

#include "rtos_qspi_flash.h"

/**
 * Open a file
 *
 * \param[in]     filename		   Filename to open
 * \param[in/out] outfile          FIL pointer
 * \param[in/out] len       	   Pointer to populate with file size
 *
 * \returns       FS_SUP_SUCCESS on success
 * 				  FS_SUP_FAIL otherwise
 */
int rtos_ff_get_file(const char* filename, FIL* outfile, unsigned int* len );

/**
 *  Initialize and mount file system
 *
 *  \param[in] qspi_flash_ctx The QSPI Flash driver context to be used
 *                            by the default implementations of the diskio
 *                            functions.
 */
void rtos_fatfs_init( rtos_qspi_flash_t *qspi_flash_ctx );

#endif /* FS_SUPPORT_H_ */
