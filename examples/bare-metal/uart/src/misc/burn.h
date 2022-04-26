// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef BURN_H_
#define BURN_H_

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

DECLARE_JOB(burn, (void));

void burn(void) {
    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);
    for(;;);
}

#endif /* BURN_H_ */
