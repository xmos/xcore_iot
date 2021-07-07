// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MULTITILE_SUPPORT_PLATFORM_H
#define MULTITILE_SUPPORT_PLATFORM_H

#if __XC__
#ifndef THIS_XCORE_TILE
#define ON_TILE(t) (t == t)
#else
#define ON_TILE(t) (THIS_XCORE_TILE == (t))
#endif
#else /* __XC__ */
#define ON_TILE(t) (!defined(THIS_XCORE_TILE) || THIS_XCORE_TILE == (t))
#endif /* __XC__ */

#include_next <platform.h>

#endif /* MULTITILE_SUPPORT_PLATFORM_H */
