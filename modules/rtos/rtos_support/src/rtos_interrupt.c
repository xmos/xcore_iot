// Copyright (c) 2021, XMOS Ltd, All rights reserved

#include "rtos_support.h"

/*
 * Ensure that these normally inline functions exist
 * when compiler optimizations are disabled.
 */
extern inline uint32_t rtos_interrupt_mask_get(void);
extern inline uint32_t rtos_interrupt_mask_all(void);
extern inline void rtos_interrupt_unmask_all(void);
extern inline void rtos_interrupt_mask_set(uint32_t mask);
extern inline uint32_t rtos_isr_running(void);
