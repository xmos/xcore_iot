// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef FS_SUPPORT_H_
#define FS_SUPPORT_H_

#include "ff.h"

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
int get_file(const char* filename, FIL* outfile, unsigned int* len );

/**
 *  Initialize and mount file system
 */
void filesystem_init( void );

#endif /* FS_SUPPORT_H_ */
