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

i2c_slave_ack_t i2c_ack_read_req(void *app_data);
i2c_slave_ack_t i2c_ack_write_req(void *app_data);
uint8_t i2c_master_req_data(void *app_data);
i2c_slave_ack_t i2c_master_sent_data(void *app_data, uint8_t data);
void i2c_stop_bit(void *app_data);
int i2c_shutdown(void *app_data);

// I2C interface ports
port_t p_scl = XS1_PORT_1N;
port_t p_sda = XS1_PORT_1O;

int main(void) {
    static const uint8_t device_addr = 0x3c;

    static i2c_callback_group_t i_i2c = {
        .ack_read_request = (ack_read_request_t) i2c_ack_read_req,
        .ack_write_request = (ack_write_request_t) i2c_ack_write_req,
        .master_requires_data = (master_requires_data_t) i2c_master_req_data,
        .master_sent_data = (master_sent_data_t) i2c_master_sent_data,
        .stop_bit = (stop_bit_t) i2c_stop_bit,
        .shutdown = (shutdown_t) i2c_shutdown,
        .app_data = NULL,
    };

    PAR_JOBS (
        PJOB(i2c_slave, (&i_i2c, p_scl, p_sda, device_addr)),
        PJOB(dummy_thread, ())
    );

    return 0;
}

void dummy_thread(void) {
    for(;;);
}

i2c_slave_ack_t i2c_ack_read_req(void *app_data) {
    return I2C_SLAVE_ACK;
}

i2c_slave_ack_t i2c_ack_write_req(void *app_data) {
    return I2C_SLAVE_ACK;
}

uint8_t i2c_master_req_data(void *app_data) {
    uint8_t data = 0x00;
    // handle read from device here
    return data;
}

i2c_slave_ack_t i2c_master_sent_data(void *app_data, uint8_t data) {
    // handle write to device here, set response to NACK for the
    // last byte of data in the transaction.
    return I2C_SLAVE_NACK;
}

void i2c_stop_bit(void *app_data) {
}

int i2c_shutdown(void *app_data) {
    return 0;
}

// end
