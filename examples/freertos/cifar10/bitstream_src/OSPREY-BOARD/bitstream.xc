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
flash_qe_config_t    flash_qe_config    = {flash_qe_location_status_reg_1, flash_qe_bit_1};

void tile0_device_instantiate(
        chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend swmem_t1_ctrl_ch)
{
    chan qspi_flash_dev_ctrl_ch;

    par {
        unsafe {
            unsafe chanend qspi_flash_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, qspi_flash_dev_ctrl_ch, null};

            device_register( qspi_flash_dev_ch, ai_dev_ch);
            soc_peripheral_hub();
        }
        {
            while (soc_tile0_bitstream_initialized() == 0);
            par {
                qspi_flash_dev(
                        bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_A],
                        null,
                        null,
                        qspi_flash_dev_ctrl_ch,
                        null,
                        swmem_t1_ctrl_ch,
                        32768, /* Number of pages in the QSPI flash */
                        &flash_ports, &flash_clock_config, &flash_qe_config);
            }
        }
    }
}

void tile1_device_instantiate(
        chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend swmem_t1_ctrl_ch)
{
    par {
        ai_dev( NULL,
                ai_dev_ch[SOC_PERIPHERAL_TO_DMA_CH],
                null,
                ai_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                swmem_t1_ctrl_ch);
    }
}
