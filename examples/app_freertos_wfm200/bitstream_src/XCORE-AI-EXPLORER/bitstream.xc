// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <stdint.h>
#include <timer.h>
//#include <quadflashlib.h>
#include <xmos_flash.h>

#include "debug_print.h"
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
#if 0
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
clock pdmclk    = on MIC_TILE: XS1_CLKBLK_3;

/*-----------------------------------------------------------*/
/* I2S defines */
/*-----------------------------------------------------------*/
#define I2S_TILE_NO 1
#define I2S_TILE tile[I2S_TILE_NO]

out buffered port:32 p_i2s_dout[1]  = {PORT_I2S_DAC_DATA};
#in port p_mclk_in1                  = PORT_MCLK_IN;
out port p_bclk                     = PORT_I2S_BCLK;
out buffered port:32 p_lrclk        = PORT_I2S_LRCLK;

clock mclk                          = on I2S_TILE: XS1_CLKBLK_3;
clock bclk                          = on I2S_TILE: XS1_CLKBLK_4;

/*-----------------------------------------------------------*/
/* I2C defines */
/*-----------------------------------------------------------*/
#if 0
port p_i2c                          = PORT_I2C;             // Bit 0: SCLK, Bit 1: SDA

#define I2C_SCL_BITPOS  0
#define I2C_SDA_BITPOS  1
#define I2C_OTHER_MASK  0

port p_rst_shared                   = PORT_SHARED_RESET;    // Bit 0: DAC_RST_N, Bit 1: ETH_RST_N
#endif
/*-----------------------------------------------------------*/
/* USB defines */
/*-----------------------------------------------------------*/
//#define USB_TILE_NO 1
//#define USB_TILE tile[USB_TILE_NO]
//
//port p_usb_tx_readyin   = PORT_USB_TX_READYIN;
//port p_usb_clk          = PORT_USB_CLK;
//port p_usb_tx_readyout  = PORT_USB_TX_READYOUT;
//port p_usb_rx_ready     = PORT_USB_RX_READY;
//port p_usb_flag0        = PORT_USB_FLAG0;
//port p_usb_flag1        = PORT_USB_FLAG1;
//port p_usb_flag2        = PORT_USB_FLAG2;
//port p_usb_txd          = PORT_USB_TXD;
//port p_usb_rxd          = PORT_USB_RXD;
#endif
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

#if 0
//Configuration for the quad spi flash
fl_QSPIPorts      flash_ports   = FLASH_PORTS;
fl_QuadDeviceSpec flash_specs[] = FLASH_SPECS;
#else
/**
 * Defines the clock/timing configuration
 * for the quad spi flash. This configuration
 * is for 50 MHz.
 */
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



#endif
void tile0_device_instantiate(
#if 0
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
#else
        void)
#endif
{
    chan t0_gpio_dev_ctrl_ch;
    chan spi_dev_ctrl_ch;
    chan qspi_flash_dev_ctrl_ch;

    par {
        unsafe {
            unsafe chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, null, null};
            unsafe chanend t0_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, t0_gpio_dev_ctrl_ch, null};
            unsafe chanend spi_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, spi_dev_ctrl_ch, null};
            unsafe chanend qspi_flash_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, qspi_flash_dev_ctrl_ch, null};

            /*
             * Must be called before device_register() so that it happens before
             * before the bitstream is "initialized" and the FreeRTOS software starts.
             */
            //fl_connectToDevice(flash_ports, flash_specs, sizeof(flash_specs)/sizeof(fl_QuadDeviceSpec));
            //flash_connect(&flash_handle, &flash_ports, flash_clock_config, flash_qe_config);

            //device_register(mic_dev_ch, i2s_dev_ch, i2c_dev_ch, t0_gpio_dev_ch, t1_gpio_dev_ch, spi_dev_ch, qspi_flash_dev_ch);
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
                        &flash_ports, &flash_clock_config, &flash_qe_config);
            }
        }
    }
}

void tile1_device_instantiate(
#if 0
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
#else
        void)
#endif
{
#if 0
    i2c_master_if i_i2c[1];
    p_rst_shared <: 0xF;

    micarray_dev_init(pdmclk, p_mclk, p_pdm_clk, p_pdm_mics);

    par {
        micarray_dev(
                bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_A],
                null,
                null,
                null,
                p_pdm_mics);

        i2c_dev(i2c_dev_ch[SOC_PERIPHERAL_CONTROL_CH], i_i2c[0]);

        [[distribute]] i2c_master_single_port(i_i2c, 1, p_i2c, 100, I2C_SCL_BITPOS, I2C_SDA_BITPOS, I2C_OTHER_MASK);

        i2s_dev(
                NULL,
                i2s_dev_ch[SOC_PERIPHERAL_TO_DMA_CH],
                i2s_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH],
                i2s_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                p_mclk_in1,
                p_lrclk, p_bclk, p_i2s_dout, 1,
                bclk);

        gpio_dev(
                NULL,
                null,
                null,
                t1_gpio_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                t1_gpio_dev_ch[SOC_PERIPHERAL_IRQ_CH]);
    }
#endif
}
