// Copyright (c) 2021, XMOS Ltd, All rights reserved
#ifndef MULTITILE_SUPPORT_PLATFORM_H
#define MULTITILE_SUPPORT_PLATFORM_H

#define ON_TILE(t) (!defined(THIS_XCORE_TILE) || THIS_XCORE_TILE == (t))

#if !(__XC__)
#define _clock_defined
#endif

#include_next "platform.h"

#endif /* MULTITILE_SUPPORT_PLATFORM_H */
