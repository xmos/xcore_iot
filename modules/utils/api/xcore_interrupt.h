// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XCORE_INTERRUPT_H_
#define XCORE_INTERRUPT_H_

#include <stdint.h>
#include <xs1.h>

#include "xcore_macros.h"

/**
 * This function gets the current interrupt mask.
 * A non-zero mask value means that interrupts are enabled.
 *
 * \returns the current interrupt mask.
 */
inline uint32_t xcore_utils_interrupt_mask_get(void)
{
    uint32_t mask;

    asm volatile(
        "getsr r11," XCORE_UTILS_STRINGIFY(XS1_SR_IEBLE_MASK) "\n"
        "mov %0, r11"
        : "=r"(mask)
        : /* no inputs */
        : /* clobbers */ "r11"
    );

    return mask;
}

/**
 * This function masks (disables) all interrupts on the
 * calling core.
 *
 * \returns the previous value of the interrupt mask.
 * This value can be passed to rtos_interrupt_mask_set()
 * to restore the interrupt mask to its previous state.
 */
inline uint32_t xcore_utils_interrupt_mask_all(void)
{
    uint32_t state;

    asm volatile(
        "getsr r11," XCORE_UTILS_STRINGIFY(XS1_SR_IEBLE_MASK) "\n"
        "mov %0, r11\n"
        "clrsr" XCORE_UTILS_STRINGIFY(XS1_SR_IEBLE_MASK) "\n"
        : "=r"(state)
        : /* no inputs */
        : /* clobbers */ "r11", "memory"
    );

    return state;
}

/**
 * This function unmasks (enables) all interrupts on the
 * calling core.
 */
inline void xcore_utils_interrupt_unmask_all(void)
{
    asm volatile(
        "setsr" XCORE_UTILS_STRINGIFY(XS1_SR_IEBLE_MASK)
        : /* no outputs */
        : /* no inputs */
        : /* clobbers */ "memory"
    );
}

/**
 * This function sets the interrupt mask.
 * A non-zero mask value unmasks (enables) interrupts.
 *
 * \param mask The value to set the interrupt mask to.
 */
inline void xcore_utils_interrupt_mask_set(uint32_t mask)
{
   if (mask != 0) {
       asm volatile(
           "setsr" XCORE_UTILS_STRINGIFY(XS1_SR_IEBLE_MASK)
           : /* no outputs */
           : /* no inputs */
           : /* clobbers */ "memory"
       );
   }
}

/*
 * This function checks to see if it is called from
 * within an ISR.
 *
 * \returns non-zero when called from within an ISR or kcall.
 */
inline uint32_t xcore_utils_isr_running(void)
{
    uint32_t kernel_mode;

    asm volatile(
        "getsr r11," XCORE_UTILS_STRINGIFY(XS1_SR_INK_MASK) "\n"
        "mov %0, r11"
        : "=r"(kernel_mode)
        : /* no inputs */
        : /* clobbers */ "r11"
    );

    return kernel_mode;
}

#endif /* XCORE_INTERRUPT_H_ */
