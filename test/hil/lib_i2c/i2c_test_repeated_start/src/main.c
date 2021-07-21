// Copyright 2014-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>
#include <xcore/interrupt.h>
#include <xcore/interrupt_wrappers.h>
#include "i2c.h"

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

port_t p_scl = XS1_PORT_1A;
port_t p_sda = XS1_PORT_1B;

DECLARE_JOB(test, (void));

void test() {
    uint8_t data[1] = {0x99};
    i2c_master_t i2c_ctx;
    i2c_master_t* i2c_ctx_ptr = &i2c_ctx;

    i2c_master_init(
            i2c_ctx_ptr,
            p_scl, 0, 0,
            p_sda, 0, 0,
            400); /* kbps */

    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);

    i2c_master_write(i2c_ctx_ptr, 0x33, data, 1, NULL, 0);
    i2c_master_write(i2c_ctx_ptr, 0x33, data, 1, NULL, 1);

    i2c_master_shutdown(i2c_ctx_ptr) ;
    exit(0);
}

DECLARE_JOB(burn, (void));

void burn(void) {
    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);
    for(;;);
}

int main(void) {
    PAR_JOBS (
        PJOB(test, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ())
    );
    return 0;
}
