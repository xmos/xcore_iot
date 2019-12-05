// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <stdint.h>
#include <timer.h>

#include "xassert.h"
#include "debug_print.h"
#include "mic_array.h"
#include "soc.h"
#include "bitstream.h"

/*-----------------------------------------------------------*/
/* Ethernet defines */
/*-----------------------------------------------------------*/
#define ETH_TILE_NO 1
#define ETH_TILE tile[ETH_TILE_NO]

port p_eth_rxclk  = PORT_ETH_RXCLK;
port p_eth_rxd    = PORT_ETH_RXD;
port p_eth_txd    = PORT_ETH_TXD;
port p_eth_rxdv   = PORT_ETH_RXDV;
port p_eth_txen   = PORT_ETH_TXEN;
port p_eth_txclk  = PORT_ETH_TXCLK;
//port p_eth_int    = PORT_ETH_INT;
port p_eth_rxerr  = PORT_ETH_RXERR;
port p_eth_timing = PORT_ETH_DUMMY;

port p_smi = PORT_SMI;

clock eth_rxclk   = on ETH_TILE: XS1_CLKBLK_1;
clock eth_txclk   = on ETH_TILE: XS1_CLKBLK_2;

otp_ports_t otp_ports = on ETH_TILE: OTP_PORTS_INITIALIZER;

/*-----------------------------------------------------------*/
/* MABS defines */
/*-----------------------------------------------------------*/
#define GPIO_TILE_NO 0
#define GPIO_TILE tile[GPIO_TILE_NO]

out port p_led0to7              = PORT_LED0_TO_7;
out port p_led8                 = PORT_LED8;
out port p_led9                 = PORT_LED9;
out port p_led10to12            = PORT_LED10_TO_12;
out port p_leds_oen             = PORT_LED_OEN;

in port p_buttons               = PORT_BUT_A_TO_D;

port p_exp_0                    = PORT_EXPANSION_1;
port p_exp_1                    = PORT_EXPANSION_3;
port p_exp_2                    = PORT_EXPANSION_5;
port p_exp_3                    = PORT_EXPANSION_7;
port p_exp_4                    = PORT_EXPANSION_9;
//port p_exp_5                    = PORT_EXPANSION_10;
port p_exp_6                    = PORT_EXPANSION_12;

/*-----------------------------------------------------------*/
/* Mic Array defines */
/*-----------------------------------------------------------*/
#define MIC_TILE_NO 0
#define MIC_TILE tile[MIC_TILE_NO]

/* Ports for the PDM microphones */
out port p_pdm_clk              = PORT_PDM_CLK;
in buffered port:32 p_pdm_mics  = PORT_PDM_DATA;

/* Clock port for the PDM mics */
in port p_mclk  = PORT_MCLK_TILE0;

/* Clock blocks for PDM mics */
clock pdmclk    = on MIC_TILE: XS1_CLKBLK_3;

/*-----------------------------------------------------------*/
/* I2S defines */
/*-----------------------------------------------------------*/
#define I2S_TILE_NO 1
#define I2S_TILE tile[I2S_TILE_NO]

out buffered port:32 p_i2s_dout[1]  = {PORT_I2S_DAC0};
in port p_mclk_in1                  = PORT_MCLK_IN;
out port p_bclk                     = PORT_I2S_BCLK;
out buffered port:32 p_lrclk        = PORT_I2S_LRCLK;

out port p_pll_sync                 = PORT_PLL_REF;
clock mclk                          = on I2S_TILE: XS1_CLKBLK_3;
clock bclk                          = on I2S_TILE: XS1_CLKBLK_4;

/*-----------------------------------------------------------*/
/* I2C defines */
/*-----------------------------------------------------------*/
port p_i2c                          = PORT_I2C;             // Bit 0: SCLK, Bit 1: SDA

#define I2C_SCL_BITPOS  0
#define I2C_SDA_BITPOS  1
#define I2C_OTHER_MASK  0

port p_rst_shared                   = PORT_SHARED_RESET;    // Bit 0: DAC_RST_N, Bit 1: ETH_RST_N

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


void tile0_device_instantiate(
        chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    chan mic_dev_to_dma_ch;
    chan t0_gpio_dev_ctrl_ch;
    chan t0_gpio_dev_irq_ch;

    micarray_dev_init(pdmclk, p_mclk, p_pdm_clk, p_pdm_mics);

    par {
        unsafe {
            unsafe chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, mic_dev_to_dma_ch, null, null};
            unsafe chanend t0_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, t0_gpio_dev_ctrl_ch, t0_gpio_dev_irq_ch};

            device_register(mic_dev_ch, eth_dev_ch, i2s_dev_ch, i2c_dev_ch, t0_gpio_dev_ch, t1_gpio_dev_ch);
            soc_peripheral_hub();
        }

        micarray_dev(
                mic_dev_to_dma_ch,
                null,
                null,
                p_pdm_mics);

        gpio_dev(
                null,
                null,
                t0_gpio_dev_ctrl_ch,
                t0_gpio_dev_irq_ch);
    }
}

void tile1_device_instantiate(
        chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    i2c_master_if i_i2c[1];
    p_rst_shared <: 0xF;

    par {
        eth_dev_smi_singleport(eth_dev_ch[SOC_PERIPHERAL_TO_DMA_CH],
                eth_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH],
                eth_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                p_eth_rxclk, p_eth_rxerr, p_eth_rxd, p_eth_rxdv,
                p_eth_txclk, p_eth_txen, p_eth_txd,
                p_eth_timing, eth_rxclk, eth_txclk,
                p_smi,
                otp_ports);

        i2c_dev(i2c_dev_ch[SOC_PERIPHERAL_CONTROL_CH], i_i2c[0]);

        [[distribute]] i2c_master_single_port(i_i2c, 1, p_i2c, 100, I2C_SCL_BITPOS, I2C_SDA_BITPOS, I2C_OTHER_MASK);

        i2s_dev(i2s_dev_ch[SOC_PERIPHERAL_TO_DMA_CH],
                i2s_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH],
                i2s_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                p_mclk_in1,
                p_lrclk, p_bclk, p_i2s_dout, 1,
                bclk);

        gpio_dev(
                null,
                null,
                t1_gpio_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                t1_gpio_dev_ch[SOC_PERIPHERAL_IRQ_CH]);
    }
}
