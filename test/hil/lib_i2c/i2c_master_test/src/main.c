// Copyright 2014-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>
#include "i2c.h"

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

/* Test 1b port SCL 1b port SDA */
#if (PORT_SETUP == 0)
port_t p_scl = XS1_PORT_1A;
port_t p_sda = XS1_PORT_1B;

uint32_t p_scl_bit_pos = 0;
uint32_t p_sda_bit_pos = 0;
#endif

/* Test 8b port shared by SCL and SDA */
#if (PORT_SETUP == 1)
port_t p_scl = XS1_PORT_8A;
port_t p_sda = XS1_PORT_8A;

uint32_t p_scl_bit_pos = 1;
uint32_t p_sda_bit_pos = 3;
#endif

/* Test 8b port SCL 8b port SDA */
#if (PORT_SETUP == 2)
port_t p_scl = XS1_PORT_8A;
port_t p_sda = XS1_PORT_8B;

uint32_t p_scl_bit_pos = 0;
uint32_t p_sda_bit_pos = 0;
#endif

/* Test 1b port SCL with overlapping 8b port SDA */
#if (PORT_SETUP == 3)
port_t p_scl = XS1_PORT_1M;
port_t p_sda = XS1_PORT_8D;

uint32_t p_scl_bit_pos = 0;
uint32_t p_sda_bit_pos = 1;
#endif

/* Test 8b port SCL with overlapping 1b port SDA */
#if (PORT_SETUP == 4)
port_t p_scl = XS1_PORT_8D;
port_t p_sda = XS1_PORT_1M;

uint32_t p_scl_bit_pos = 1;
uint32_t p_sda_bit_pos = 0;
#endif

// Test the following pairs of operations:
// write -> read
// read  -> read
// read  -> write
// write -> write
enum {
    TEST_WRITE_1 = 0,
    TEST_READ_1,
    TEST_READ_2,
    TEST_WRITE_2,
    TEST_WRITE_3,
    NUM_TESTS
};

static const char* ack_str(i2c_res_t ack)
{
    return (ack == I2C_ACK) ? "ack" : "nack";
}

#define MAX_DATA_BYTES 3

DECLARE_JOB(test, (void));

void test() {
    // Have separate data arrays so that everything can be setup before starting
    uint8_t data_write_1[MAX_DATA_BYTES] = {0};
    uint8_t data_write_2[MAX_DATA_BYTES] = {0};
    uint8_t data_write_3[MAX_DATA_BYTES] = {0};
    uint8_t data_read_1[MAX_DATA_BYTES] = {0};
    uint8_t data_read_2[MAX_DATA_BYTES] = {0};
    i2c_res_t acks[NUM_TESTS] = {0};
    size_t n1 = 0;
    size_t n2 = 0;
    size_t n3 = 0;

    const int do_stop = STOP ? 1 : 0;

    i2c_master_t i2c_ctx;
    i2c_master_t* i2c_ctx_ptr = &i2c_ctx;

    i2c_master_init(
            i2c_ctx_ptr,
            p_scl, p_scl_bit_pos, 0,
            p_sda, p_sda_bit_pos, 0,
            SPEED); /* kbps */

    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);

    // Setup all data to be written
    data_write_1[0] = 0x90; data_write_1[1] = 0xfe;
    data_write_2[0] = 0xff; data_write_2[1] = 0x00; data_write_2[2] = 0xaa;
    data_write_3[0] = 0xee;

    // Execute all bus operations
    if (ENABLE_TX) {
        acks[TEST_WRITE_1] = i2c_master_write(i2c_ctx_ptr, 0x3c, data_write_1, 2, &n1, do_stop);
    }
    if (ENABLE_RX) {
        acks[TEST_READ_1] = i2c_master_read(i2c_ctx_ptr, 0x22, data_read_1, 2, do_stop);
        acks[TEST_READ_2] = i2c_master_read(i2c_ctx_ptr, 0x22, data_read_2, 1, do_stop);
    }
    if (ENABLE_TX) {
        acks[TEST_WRITE_2] = i2c_master_write(i2c_ctx_ptr, 0x7b, data_write_2, 3, &n2, do_stop);
        acks[TEST_WRITE_3] = i2c_master_write(i2c_ctx_ptr, 0x31, data_write_3, 1, &n3, do_stop);
    }

    // Print out results after all the data transactions have finished
    if (ENABLE_TX) {
        printf("xCORE got %s, %d\n", ack_str(acks[TEST_WRITE_1]), n1);
        printf("xCORE got %s, %d\n", ack_str(acks[TEST_WRITE_2]), n2);
        printf("xCORE got %s, %d\n", ack_str(acks[TEST_WRITE_3]), n3);
    }

    if (ENABLE_RX) {
        printf("xCORE got %s\n", ack_str(acks[TEST_READ_1]));
        printf("xCORE received: 0x%X, 0x%X\n", data_read_1[0], data_read_1[1]);
        printf("xCORE got %s\n", ack_str(acks[TEST_READ_2]));
        printf("xCORE received: 0x%X\n", data_read_2[0]);
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
