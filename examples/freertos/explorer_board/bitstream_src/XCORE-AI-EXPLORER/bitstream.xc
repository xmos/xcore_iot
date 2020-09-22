// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <platform.h>
#include <stdint.h>
#include <timer.h>
#include <xmos_flash.h>

#include "xassert.h"
#include "mic_array.h"
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
/* Mic Array defines */
/*-----------------------------------------------------------*/
#define MIC_TILE_NO 1
#define MIC_TILE tile[MIC_TILE_NO]

/* Ports for the PDM microphones */
out port p_pdm_clk              = PORT_PDM_CLK;
in buffered port:32 p_pdm_mics  = PORT_PDM_DATA;

/* Clock port for the PDM mics */
in port p_mclk  = PORT_MCLK_IN;

/* Clock blocks for PDM mics */
clock pdmclk    = on MIC_TILE : XS1_CLKBLK_1;
clock pdmclk2   = on MIC_TILE : XS1_CLKBLK_2;

/* Setup internal clock to be mic PLL */
clock mclk_internal = on MIC_TILE : XS1_CLKBLK_3;

/*-----------------------------------------------------------*/
/* I2S defines */
/*-----------------------------------------------------------*/
#define I2S_TILE_NO 1
#define I2S_TILE tile[I2S_TILE_NO]

out buffered port:32 p_i2s_dout[1]  = {PORT_I2S_DAC_DATA};
out port p_bclk                     = PORT_I2S_BCLK;
out buffered port:32 p_lrclk        = PORT_I2S_LRCLK;

clock mclk                          = on I2S_TILE: XS1_CLKBLK_4;
clock bclk                          = on I2S_TILE: XS1_CLKBLK_5;

port p_rst_shared                   = PORT_CODEC_RST_N;
/*-----------------------------------------------------------*/
/* I2C defines */
/*-----------------------------------------------------------*/
port p_i2c_scl 					      = PORT_I2C_SCL;
port p_i2c_sda 						  = PORT_I2C_SDA;

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

/*-----------------------------------------------------------*/
/* Helpers to setup a clock as a PLL */
/*-----------------------------------------------------------*/

// 24MHz in, 24.576MHz out, integer mode
// Found exact solution:   IN  24000000.0, OUT  24576000.0, VCO 2457600000.0, RD  5, FD  512                       , OD  5, FOD   10
#define APP_PLL_DISABLE 0x0201FF04
#define APP_PLL_CTL_0   0x0A01FF04
#define APP_PLL_DIV_0   0x80000004
#define APP_PLL_FRAC_0  0x00000000

void set_app_pll(void) {
	write_node_config_reg(tile[0], XS1_SSWITCH_SS_APP_PLL_CTL_NUM,            APP_PLL_DISABLE);
	delay_milliseconds(1);
	write_node_config_reg(tile[0], XS1_SSWITCH_SS_APP_PLL_CTL_NUM,            APP_PLL_CTL_0);
	write_node_config_reg(tile[0], XS1_SSWITCH_SS_APP_PLL_CTL_NUM,            APP_PLL_CTL_0);
	write_node_config_reg(tile[0], XS1_SSWITCH_SS_APP_PLL_FRAC_N_DIVIDER_NUM, APP_PLL_FRAC_0);
	write_node_config_reg(tile[0], XS1_SSWITCH_SS_APP_CLK_DIVIDER_NUM,        APP_PLL_DIV_0);
}

void tile0_device_instantiate(
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    chan t0_gpio_dev_ctrl_ch;
    chan spi_dev_ctrl_ch;
    chan i2c_dev_ctrl_ch;
    chan qspi_flash_dev_ctrl_ch;
    i2c_master_if i_i2c[1];

    par {
        unsafe {
            unsafe chanend t0_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, t0_gpio_dev_ctrl_ch, null};
            unsafe chanend spi_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, spi_dev_ctrl_ch, null};
            unsafe chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, i2c_dev_ctrl_ch, null};
            unsafe chanend qspi_flash_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, qspi_flash_dev_ctrl_ch, null};

            /*
             * Must be called before device_register() so that it happens before
             * before the bitstream is "initialized" and the FreeRTOS software starts.
             */
            device_register(mic_dev_ch, i2s_dev_ch, i2c_dev_ch, t0_gpio_dev_ch, t1_gpio_dev_ch, spi_dev_ch, qspi_flash_dev_ch);
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

void tile1_device_instantiate(
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    p_rst_shared <: 0xF;
    set_app_pll();

    micarray_dev_init(pdmclk, pdmclk2, p_mclk, p_pdm_clk, p_pdm_mics);

    par {
    	micarray_dev_1b(
                NULL,
				mic_dev_ch[SOC_PERIPHERAL_TO_DMA_CH],
				mic_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH],
				mic_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                p_pdm_mics);

        i2s_dev(
                NULL,
                i2s_dev_ch[SOC_PERIPHERAL_TO_DMA_CH],
                i2s_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH],
                i2s_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                p_mclk,
                p_lrclk, p_bclk, p_i2s_dout, 1,
                bclk);

        gpio_dev(
                NULL,
                null,
                null,
                t1_gpio_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                t1_gpio_dev_ch[SOC_PERIPHERAL_IRQ_CH]);
    }
}
