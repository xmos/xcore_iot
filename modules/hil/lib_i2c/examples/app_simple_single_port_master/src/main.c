// Copyright 2018-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* A simple application example used for code snippets in the library
 * documentation.
 */

#include <xs1.h>
#include <stdio.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>
#include "i2c.h"

DECLARE_JOB(dummy_thread, (void));
DECLARE_JOB(my_application, (i2c_master_t*, uint8_t));

// I2C interface ports
port_t p_i2c = XS1_PORT_4C;

int main(void) {
    static const uint8_t target_device_addr = 0x3c;
    static i2c_master_t i2c_ctx;

    i2c_master_init(
            &i2c_ctx,
            p_i2c, 1, 0,
            p_i2c, 3, 0,
            100); /* kbps */

    PAR_JOBS (
        PJOB(my_application, (&i2c_ctx, target_device_addr)),
        PJOB(dummy_thread, ())
    );

    return 0;
}

void dummy_thread(void) {
    for(;;);
}

void my_application(i2c_master_t* ctx, uint8_t target_device_addr) {
    uint8_t data[2] = {0};

    i2c_master_read(ctx, target_device_addr, data, 2, 1);
    printf("Read data %d, %d from the bus.\n", data[0], data[1]);
}

// end
