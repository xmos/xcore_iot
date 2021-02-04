// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#include <platform.h>
#include <stdint.h>

#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

/*-----------------------------------------------------------*/
/* SPI defines */
/*-----------------------------------------------------------*/
#define SPI_TILE_NO 0
#define SPI_TILE tile[SPI_TILE_NO]

spi_fast_ports spi_ctx = {
        on SPI_TILE: XS1_PORT_1L,  /* SCK  */
        on SPI_TILE: XS1_PORT_1M,  /* MISO */
        on SPI_TILE: XS1_PORT_1J,  /* MOSI */
        on SPI_TILE: XS1_PORT_1A,  /* CS   */
        on SPI_TILE: XS1_CLKBLK_1, /* clock block */
        0, /* CS is at bit 1 on its port */
};

/*-----------------------------------------------------------*/
/* I2C defines */
/*-----------------------------------------------------------*/
port p_i2c_scl = PORT_I2C_SCL;
port p_i2c_sda = PORT_I2C_SDA;

void tile0_device_instantiate(
        chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend swmem_ctrl_ch)
{
    chan t0_gpio_dev_ctrl_ch;
    chan spi_dev_ctrl_ch;
    chan i2c_dev_ctrl_ch;
    i2c_master_if i_i2c[1];

    par {
        unsafe {
            unsafe chanend t0_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, t0_gpio_dev_ctrl_ch, null};
            unsafe chanend spi_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, spi_dev_ctrl_ch, null};
            unsafe chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, i2c_dev_ctrl_ch, null};

            device_register(i2c_dev_ch, t0_gpio_dev_ch, spi_dev_ch, ai_dev_ch);
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

                i2c_dev(i2c_dev_ctrl_ch, i_i2c[0]);

                [[distribute]] i2c_master(i_i2c, 1, p_i2c_scl, p_i2c_sda, 100);

                spi_master_dev(
                        bitstream_spi_devices[BITSTREAM_SPI_DEVICE_A],
                        null,
                        null,
                        spi_dev_ctrl_ch,
                        spi_ctx);
            }
        }
    }
}

void tile1_device_instantiate(
        chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend swmem_ctrl_ch)
{
    par {
        ai_dev( NULL,
                ai_dev_ch[SOC_PERIPHERAL_TO_DMA_CH],
                ai_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH],
                ai_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                swmem_ctrl_ch);
    }
}
