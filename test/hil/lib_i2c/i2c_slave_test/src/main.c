// Copyright 2014-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <string.h>
#include <print.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>
#include <xcore/interrupt.h>
#include <xcore/interrupt_wrappers.h>
#include "i2c.h"

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

#define DEVICE_ADDR  0x3c

port_t p_scl = XS1_PORT_1A;
port_t p_sda = XS1_PORT_1B;

static int i = 0;
static int ack_index = 0;
uint8_t test_data[] = { 0xff, 0x01, 0x99, 0x20, 0x33, 0xee };
int ack_sequence[7] = {I2C_SLAVE_ACK, I2C_SLAVE_ACK, I2C_SLAVE_NACK,
                       I2C_SLAVE_NACK,
                       I2C_SLAVE_ACK, I2C_SLAVE_NACK};

I2C_CALLBACK_ATTR
i2c_slave_ack_t i2c_ack_read_req(void *app_data) {
    printstr("xCORE got start of read transaction\n");
    return I2C_SLAVE_ACK;
}

I2C_CALLBACK_ATTR
i2c_slave_ack_t i2c_ack_write_req(void *app_data) {
    printstr("xCORE got start of write transaction\n");
    return I2C_SLAVE_ACK;
}

I2C_CALLBACK_ATTR
uint8_t i2c_master_req_data(void *app_data) {
    int data = test_data[i];
    printf("xCORE sending: 0x%X\n", data);
    i++;
    if (i >= sizeof(test_data)) {
        i = 0;
    }
    return data;
}

I2C_CALLBACK_ATTR
i2c_slave_ack_t i2c_master_sent_data(void *app_data, uint8_t data) {
    printf("xCORE got data: 0x%X\n", data);
    if (data == 0xff) {
        _Exit(1);
    }
    return ack_sequence[ack_index++];
}

I2C_CALLBACK_ATTR
void i2c_stop_bit(void *app_data) {
    // The stop_bit function is timing critical. Needs to use printstr to meet
    // timing and detect the start bit
    printstr("xCORE got stop bit\n");
}

I2C_CALLBACK_ATTR
int i2c_shutdown(void *app_data) {
    return 0;
}


DECLARE_JOB(burn, (void));

void burn(void) {
    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);
    for(;;);
}

int main(void) {
    i2c_callback_group_t i_i2c = {
        .ack_read_request = (ack_read_request_t) i2c_ack_read_req,
        .ack_write_request = (ack_write_request_t) i2c_ack_write_req,
        .master_requires_data = (master_requires_data_t) i2c_master_req_data,
        .master_sent_data = (master_sent_data_t) i2c_master_sent_data,
        .stop_bit = (stop_bit_t) i2c_stop_bit,
        .shutdown = (shutdown_t) i2c_shutdown,
        .app_data = NULL,
    };

    PAR_JOBS (
        PJOB(i2c_slave, (&i_i2c, p_scl, p_sda, DEVICE_ADDR)),
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
