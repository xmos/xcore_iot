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

enum {
    TEST_WRITE_1 = 0,
    TEST_WRITE_2,
    TEST_WRITE_3,
    TEST_WRITE_4,
    NUM_WRITE_TESTS
};

enum {
    TEST_READ_1 = 0,
    TEST_READ_2,
    TEST_READ_3,
    TEST_READ_4,
    NUM_READ_TESTS
};

DECLARE_JOB(test, (void));

void test() {
    i2c_regop_res_t write_results[NUM_WRITE_TESTS];
    i2c_regop_res_t read_results[NUM_READ_TESTS];

    i2c_master_t i2c_ctx;
    i2c_master_t* i2c_ctx_ptr = &i2c_ctx;

    i2c_master_init(
            i2c_ctx_ptr,
            p_scl, 0, 0,
            p_sda, 0, 0,
            400); /* kbps */

    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);

    // Test register writing
    write_results[TEST_WRITE_1] = write_reg(i2c_ctx_ptr, 0x44, 0x07, 0x12);
    write_results[TEST_WRITE_2] = write_reg8_addr16(i2c_ctx_ptr, 0x22, 0xfe99, 0x12);
    write_results[TEST_WRITE_3] = write_reg16(i2c_ctx_ptr, 0x33, 0xabcd, 0x12a3);
    write_results[TEST_WRITE_4] = write_reg16_addr8(i2c_ctx_ptr, 0x11, 0xef, 0x4567);

    // Test register reading
    unsigned vals[NUM_READ_TESTS] = {0};
    vals[TEST_READ_1] = read_reg(i2c_ctx_ptr, 0x44, 0x33, &read_results[TEST_READ_1]);
    vals[TEST_READ_2] = read_reg8_addr16(i2c_ctx_ptr, 0x45, 0xa321, &read_results[TEST_READ_2]);
    vals[TEST_READ_3] = read_reg16(i2c_ctx_ptr, 0x46, 0x3399, &read_results[TEST_READ_3]);
    vals[TEST_READ_4] = read_reg16_addr8(i2c_ctx_ptr, 0x47, 0x22, &read_results[TEST_READ_4]);

    // Print all the results
    for (size_t i = 0; i < NUM_WRITE_TESTS; ++i) {
        printf(write_results[i] == I2C_REGOP_SUCCESS ? "ACK\n" : "NACK\n");
    }

    for (size_t i = 0; i < NUM_READ_TESTS; ++i) {
        printf(read_results[i] == I2C_REGOP_SUCCESS ? "ACK\n" : "NACK\n");
        printf("val=%X\n", vals[i]);
    }

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
