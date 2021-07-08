// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xcore/triggerable.h>
#include <xcore/hwtimer.h>
#include <xcore/assert.h>
#include "xclib.h"
#include "i2c.h"

#ifndef LIBXCORE_HWTIMER_HAS_REFERENCE_TIME
#error This library requires reference time
#endif

enum i2c_slave_state {
    WAITING_FOR_START_OR_STOP,
    READING_ADDR,
    ACK_ADDR,
    ACK_WAIT_HIGH,
    ACK_WAIT_LOW,
    IGNORE_ACK,
    MASTER_WRITE,
    MASTER_READ
};

static inline void ensure_setup_time()
{
    // The I2C spec requires a 100ns setup time
    uint32_t start_time = get_reference_time();
    while ((get_reference_time() - start_time) < 10) {;}
}

void i2c_slave(const i2c_callback_group_t *const i2c_cbg,
               port_t p_scl,
               port_t p_sda,
               uint8_t device_addr) {

    enum i2c_slave_state state = WAITING_FOR_START_OR_STOP;
    enum i2c_slave_state next_state = WAITING_FOR_START_OR_STOP;
    int sda_val = 0;
    int scl_val = 0;
    int bitnum = 0;
    int rw = 0;
    int stop_bit_check = 0;
    int ignore_stop_bit = 1;
    int data;
    int val;
    int bit;
    i2c_slave_ack_t ack;

    port_enable(p_scl);
    port_enable(p_sda);

    TRIGGERABLE_SETUP_EVENT_VECTOR(p_scl, event_scl);
    TRIGGERABLE_SETUP_EVENT_VECTOR(p_sda, event_sda);

    /* Wait to start until SDA is high */
    (void) port_in_when_pinseq(p_sda, PORT_UNBUFFERED, 1);

    while (1) {
        triggerable_disable_all();

        /* Check for shutdown request */
        if (i2c_cbg->shutdown(i2c_cbg->app_data)) {
            break;  // TODO: add event for shutdown
        }

        if (state == WAITING_FOR_START_OR_STOP || stop_bit_check) {
            port_set_trigger_in_equal(p_sda, sda_val);
            triggerable_enable_trigger(p_sda);
        }

        if (state != WAITING_FOR_START_OR_STOP) {
            port_set_trigger_in_equal(p_scl, scl_val);
            triggerable_enable_trigger(p_scl);
        }

        TRIGGERABLE_WAIT_EVENT(event_scl, event_sda);

        {
        event_scl:
            {
                port_clear_trigger_in(p_scl);
                port_clear_trigger_in(p_sda);
                switch (state) {
                default:
                    xassert(0); /* Unhandled state*/
                    break;
                case READING_ADDR:
                    /* Wait for clock to go back to high */
                    if (scl_val == 0) {
                        scl_val = 1;
                        break;
                    }

                    bit = port_in(p_sda);
                    if (bitnum < 7) {
                        data = (data << 1) | bit;
                        bitnum++;
                        scl_val = 0;
                        break;
                    }

                    // We have gathered the whole device address sent by the master
                    if (data != device_addr) {
                        state = IGNORE_ACK;
                    } else {
                        state = ACK_ADDR;
                        rw = bit;
                    }
                    scl_val = 0;
                    break;

                case IGNORE_ACK:
                    // This request is not for us, ignore the ACK
                    next_state = WAITING_FOR_START_OR_STOP;
                    scl_val = 1;
                    state = ACK_WAIT_HIGH;
                    break;

                case ACK_ADDR:
                    // Stretch clock (hold low) while application code is called
                    port_out(p_scl, 0);

                    // Callback to the application to determine whether to ACK
                    // or NACK the address.
                    if (rw) {
                        ack = i2c_cbg->ack_read_request(i2c_cbg->app_data);
                    } else {
                        ack = i2c_cbg->ack_write_request(i2c_cbg->app_data);
                    }

                    ignore_stop_bit = 0;
                    if (ack == I2C_SLAVE_NACK) {
                        // Release the data line so that it is pulled high
                        (void) port_in(p_sda);
                        next_state = WAITING_FOR_START_OR_STOP;
                    } else {
                        // Drive the ACK low
                        port_out(p_sda, 0);
                        if (rw) {
                            next_state = MASTER_READ;
                        } else {
                            next_state = MASTER_WRITE;
                        }
                    }
                    scl_val = 1;
                    state = ACK_WAIT_HIGH;

                    ensure_setup_time();

                    // Release the clock
                    (void) port_in(p_scl);
                    break;

                case ACK_WAIT_HIGH:
                    // Rising edge of clock, hold ack to the falling edge
                    state = ACK_WAIT_LOW;
                    scl_val = 0;
                    break;

                case ACK_WAIT_LOW:
                    // ACK done, release the data line
                    (void) port_in(p_sda);
                    if (next_state == MASTER_READ) {
                        scl_val = 0;
                    } else if (next_state == MASTER_WRITE) {
                        data = 0;
                        scl_val = 1;
                    } else { // WAITING_FOR_START_OR_STOP
                        sda_val = 0;
                    }
                    state = next_state;
                    bitnum = 0;
                    break;

                case MASTER_READ:
                    if (scl_val == 1) {
                        // Rising edge
                        if (bitnum == 8) {
                            // Sample ack from master
                            bit = port_in(p_sda);
                            if (bit) {
                                // Master has NACKed so the transaction is finished
                                state = WAITING_FOR_START_OR_STOP;
                                sda_val = 0;
                            } else {
                                bitnum = 0;
                                scl_val = 0;
                            }
                        } else {
                            // Wait for next falling edge
                            scl_val = 0;
                            bitnum++;
                        }
                    } else {
                        // Falling edge, drive data
                        if (bitnum < 8) {
                            if (bitnum == 0) {
                                // Stretch clock (hold low) while application code is called
                                port_out(p_scl, 0);
                                data = i2c_cbg->master_requires_data(i2c_cbg->app_data);
                                // Data is transmitted MSB first
                                data = bitrev(data) >> 24;

                                // Send first bit of data
                                port_out(p_sda, data & 0x1);

                                ensure_setup_time();

                                // Release the clock
                                (void) port_in(p_scl);
                            } else {
                                port_out(p_sda, data & 0x1);
                            }
                            data >>= 1;
                        } else {
                            // Release the bus for the master to be able to ACK/NACK
                            (void) port_in(p_sda);
                        }
                        scl_val = 1;
                    }
                    break;

                case MASTER_WRITE:
                    if (scl_val == 1) {
                        // Rising edge
                        bit = port_in(p_sda);
                        data = (data << 1) | (bit & 0x1);
                        if (bitnum == 0) {
                            if (bit) {
                                sda_val = 0;
                            } else {
                                sda_val = 1;
                            }
                            // First bit could be a start or stop bit
                            stop_bit_check = 1;
                        }
                        scl_val = 0;
                        bitnum++;
                    } else {
                        // Falling edge

                        // Not a start or stop bit
                        stop_bit_check = 0;

                        if (bitnum == 8) {
                            // Stretch clock (hold low) while application code is called
                            port_out(p_scl, 0);
                            ack = i2c_cbg->master_sent_data(i2c_cbg->app_data, data);
                            if (ack == I2C_SLAVE_NACK) {
                                // Release the data bus so it is pulled high to signal NACK
                                (void) port_in(p_sda);
                            } else {
                                // Drive data bus low to signal ACK
                                port_out(p_sda, 0);
                            }
                            state = ACK_WAIT_HIGH;

                            ensure_setup_time();

                            // Release the clock
                            (void) port_in(p_scl);
                        }
                        scl_val = 1;
                    }
                    break;
                }
            }
            continue;
        }

        {
        event_sda:
            port_clear_trigger_in(p_scl);
            port_clear_trigger_in(p_sda);
            {
                val = port_in(p_scl);
                if (sda_val == 1) {
                    /* SDA has transitioned low to high,
                     * so check SCL for stop bit */
                    if (val) {
                        if (!ignore_stop_bit && i2c_cbg->stop_bit != NULL) {
                            // Hold the clock low while application code is called.
                            // If the master supports multi-master, then this should
                            // ensure that it does not begin another transaction until
                            // the application code here has returned.
                            port_out(p_scl, 0);

                            i2c_cbg->stop_bit(i2c_cbg->app_data);

                            // Release the clock
                            (void) port_in(p_scl);
                        }
                        state = WAITING_FOR_START_OR_STOP;
                        ignore_stop_bit = 1;
                        stop_bit_check = 0;
                    }
                    sda_val = 0;
                } else {
                    /* SDA has transitioned high to low,
                     * so check SCL for start bit */
                    if (val) {
                        state = READING_ADDR;
                        bitnum = 0;
                        data = 0;
                        scl_val = 0;
                        stop_bit_check = 0;
                    } else {
                        sda_val = 1;
                    }
                }
            }
            continue;
        }
    }

    port_disable(p_scl);
    port_disable(p_sda);
}
