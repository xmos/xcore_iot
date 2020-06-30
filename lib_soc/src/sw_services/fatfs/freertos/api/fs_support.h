// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef FS_SUPPORT_H_
#define FS_SUPPORT_H_

#include "ff.h"

int get_file(const char* filename, FIL* outfile, unsigned int* len );

void filesystem_init( void );

#endif /* FS_SUPPORT_H_ */
