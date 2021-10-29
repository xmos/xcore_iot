// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XCORE_MACROS_H_
#define XCORE_MACROS_H_

#define XCORE_UTILS_STRINGIFY_I(...) #__VA_ARGS__
#define XCORE_UTILS_STRINGIFY(...) XCORE_UTILS_STRINGIFY_I(__VA_ARGS__)
/*
 * Inserts a compile time memory barrier
 */
#define XCORE_UTILS_MEMORY_BARRIER() asm volatile( "" ::: "memory" )

/*
 * Returns the number of 32-bit stack words required by the given thread entry
 * function.
 *
 * This will not just "work" if there is any recursion or function pointers in
 * the thread.  For function pointers the fptrgroup attribute may be used.
 * For recursive functions, the "stackfunction" #pragma may be used.
 */
#define XCORE_UTILS_THREAD_STACK_SIZE(thread_entry)                            \
    ({                                                                         \
        uint32_t stack_size;                                                   \
        asm volatile (                                                         \
                "ldc %0, " XCORE_UTILS_STRINGIFY(thread_entry) ".nstackwords"  \
                : "=r"(stack_size) /* output 0 is stack_size */                \
                : /* there are no inputs */                                    \
                : /* nothing gets clobbered */                                 \
        );                                                                     \
        stack_size;                                                            \
    })

#endif /* XCORE_MACROS_H_ */
