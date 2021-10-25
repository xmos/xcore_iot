// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <xs1.h>

#include "xcore_interrupt.h"

/*
 * Ensure that these normally inline functions exist
 * when compiler optimizations are disabled.
 */
extern inline uint32_t xcore_utils_interrupt_mask_get(void);
extern inline uint32_t xcore_utils_interrupt_mask_all(void);
extern inline void xcore_utils_interrupt_unmask_all(void);
extern inline void xcore_utils_interrupt_mask_set(uint32_t mask);
extern inline uint32_t xcore_utils_isr_running(void);
