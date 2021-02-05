// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1
#ifndef MULTITILE_SUPPORT_PLATFORM_H
#define MULTITILE_SUPPORT_PLATFORM_H

#define ON_TILE(t) (!defined(THIS_XCORE_TILE) || THIS_XCORE_TILE == (t))

#if !(__XC__)
#define _clock_defined
#endif

#include_next <platform.h>

#endif /* MULTITILE_SUPPORT_PLATFORM_H */
