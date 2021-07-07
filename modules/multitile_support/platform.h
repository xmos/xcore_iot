// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MULTITILE_SUPPORT_PLATFORM_H
#define MULTITILE_SUPPORT_PLATFORM_H

#ifdef THIS_XCORE_TILE
#define ON_TILE(t) (THIS_XCORE_TILE == (t))
#else
#error "THIS_XCORE_TILE not defined!"
#endif

#include_next <platform.h>

#endif /* MULTITILE_SUPPORT_PLATFORM_H */
