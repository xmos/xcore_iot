// Copyright (c) 2016-2020, XMOS Ltd, All rights reserved
#ifndef SPI_FAST_H_
#define SPI_FAST_H_

#include <stdint.h>

#ifndef __XC__
#include <xcore/port.h>
#include <xcore/clock.h>
#else
#include <xs1_clock.h>
#endif

typedef struct {
    /****************************************/
    /*** Must be declared at compile time ***/
    /****************************************/
#ifdef __XC__
    out buffered port:32 clk;
    in buffered port:32 miso;
    out buffered port:32 mosi;
    out port cs;
    clock cb;
#else
    port_t clk;
    port_t miso;
    port_t mosi;
    port_t cs;
    xclock_t cb;
#endif
    unsigned cs_port_bit;
    /****************************************/

    /****************************************/
    /***      For internal use only       ***/
    /****************************************/
    unsigned cs_to_data_delay_ticks;
    unsigned byte_setup_ticks;
    unsigned data_to_cs_delay_ticks;
    unsigned clock_bits;
    unsigned clock_delay;
    /****************************************/

    /****************************************/
    /*** May be set at runtime. Must call ***/
    /*** spi_fast_init() after adjusting. ***/
    /****************************************/
    unsigned cpol;
    unsigned cpha;
    unsigned clock_divide; /* SCK Frequency = 100 MHz / (2 * clock_divide) / 2 */
    unsigned cs_to_data_delay_ns;
    unsigned byte_setup_ns;
    unsigned data_to_cs_delay_ns;
    /****************************************/
} spi_fast_ports;

typedef enum port_time_mode_t {
    SPI_CS_DRIVE_NOW,
    SPI_CS_DRIVE_AT_TIME,
    SPI_CS_GET_TIMESTAMP
} port_time_mode_t;

typedef enum spi_direction_t {
    SPI_READ,
    SPI_WRITE,
    SPI_READ_WRITE
} spi_direction_t;

void spi_fast_init(spi_fast_ports *p);

void spi_fast(unsigned num_bytes, char *buffer, spi_fast_ports *p, spi_direction_t direction);

void drive_cs_port_now(spi_fast_ports *p,
                       uint32_t bit_value);

void drive_cs_port_at_time(spi_fast_ports *p,
                           uint32_t bit_value,
                           uint32_t time);

void drive_cs_port_get_time(spi_fast_ports *p,
                            uint32_t bit_value,
                            uint32_t *time);

#endif /* SPI_FAST_H_ */

