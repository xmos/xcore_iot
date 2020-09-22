// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <platform.h>
#include <stdint.h>
#include <timer.h>
#include <xmos_flash.h>

#include "xassert.h"
#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

/*-----------------------------------------------------------*/
/* SPI defines */
/*-----------------------------------------------------------*/
#define SPI_TILE_NO 0
#define SPI_TILE tile[SPI_TILE_NO]

spi_fast_ports spi_ctx = {
        WIFI_CLK, /* SCK  */
        WIFI_MISO, /* MISO */
        WIFI_MOSI, /* MOSI */
        WIFI_CS_N, /* CS   */
        on SPI_TILE: XS1_CLKBLK_1, /* clock block */
        1, /* CS is at bit 1 on its port */
};

/*-----------------------------------------------------------*/
/* Quad SPI Flash defines */
/*-----------------------------------------------------------*/
/**
 * Defines the ports and clock block
 * to use for the quad spi flash.
 */
#define FLASH_PORTS {           \
    PORT_SQI_CS,                \
    PORT_SQI_SCLK,              \
    PORT_SQI_SIO,               \
    on tile[0]: XS1_CLKBLK_2    \
}

/**
 * Defines the supported flash chips.
 */
#define FLASH_SPECS {             \
    FL_QUADDEVICE_SPANSION_S25FL116K \
}

#define FLASH_CLOCK_CONFIG {        \
    flash_clock_reference,          \
    0,                              \
    1,                              \
    flash_clock_input_edge_plusone, \
    flash_port_pad_delay_1          \
}

//Configuration for the quad spi flash
flash_handle_t       flash_handle;
flash_ports_t        flash_ports        = FLASH_PORTS;
flash_clock_config_t flash_clock_config = FLASH_CLOCK_CONFIG;
flash_qe_config_t    flash_qe_config    = {flash_qe_location_status_reg_0, flash_qe_bit_6};


void tile0_device_instantiate()
{
    chan t0_gpio_dev_ctrl_ch;
    chan spi_dev_ctrl_ch;
    chan qspi_flash_dev_ctrl_ch;

    par {
        unsafe {
            unsafe chanend t0_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, t0_gpio_dev_ctrl_ch, null};
            unsafe chanend spi_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, spi_dev_ctrl_ch, null};
            unsafe chanend qspi_flash_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, qspi_flash_dev_ctrl_ch, null};

            /*
             * Must be called before device_register() so that it happens before
             * before the bitstream is "initialized" and the FreeRTOS software starts.
             */
            device_register(t0_gpio_dev_ch, spi_dev_ch, qspi_flash_dev_ch);
            soc_peripheral_hub();
        }
        {
            while (soc_tile0_bitstream_initialized() == 0);
            par {
                gpio_dev(
                        bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_A],
                        null,
                        null,
                        t0_gpio_dev_ctrl_ch,
                        null);

                spi_master_dev(
                        bitstream_spi_devices[BITSTREAM_SPI_DEVICE_A],
                        null,
                        null,
                        spi_dev_ctrl_ch,
                        spi_ctx);

                qspi_flash_dev(
                        bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_A],
                        null,
                        null,
                        qspi_flash_dev_ctrl_ch,
                        null,
                        null,
                        16384, /* Number of pages in the QSPI flash */
                        &flash_ports, &flash_clock_config, &flash_qe_config);
            }
        }
    }
}

void tile1_device_instantiate()
{
}
