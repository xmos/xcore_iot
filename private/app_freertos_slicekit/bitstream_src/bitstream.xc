// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <stdint.h>
#include <timer.h>

#include "xassert.h"
#include "debug_print.h"
#include "mic_array.h"
#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

/*-----------------------------------------------------------*/
/* Ethernet slice defines */
/*-----------------------------------------------------------*/
#define ETH_TILE_NO 1
#define ETH_TILE tile[ETH_TILE_NO]

port p_eth_rxclk  = PORT_ETH_RXCLK;
port p_eth_rxd    = PORT_ETH_RXD;
port p_eth_txd    = PORT_ETH_TXD;
port p_eth_rxdv   = PORT_ETH_RXDV;
port p_eth_txen   = PORT_ETH_TXEN;
port p_eth_txclk  = PORT_ETH_TXCLK;
port p_eth_int    = PORT_ETH_INT;
port p_eth_rxerr  = PORT_ETH_RXERR;
port p_eth_timing = PORT_ETH_DUMMY;

port p_smi_mdc = PORT_SMI_MDC;
port p_smi_mdio = PORT_SMI_MDIO;

clock eth_rxclk   = on ETH_TILE: XS1_CLKBLK_1;
clock eth_txclk   = on ETH_TILE: XS1_CLKBLK_2;

otp_ports_t otp_ports = on ETH_TILE: OTP_PORTS_INITIALIZER;


/*-----------------------------------------------------------*/
/* Mic Array slice defines */
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

out port p_pll_sync = PORT_PLL_REF;

/* Setup internal clock to be mic PLL */
clock mclk_internal = on MIC_TILE : XS1_CLKBLK_4;

#define PLL_NOM  0xC003FF18


/*-----------------------------------------------------------*/
/* SDRAM slice defines */
/*-----------------------------------------------------------*/
#define SDRAM_TILE_NO 1
#define SDRAM_TILE tile[SDRAM_TILE_NO]

out buffered port:32   p_sdram_dq_ah = PORT_SDRAM_DQ_AH;
out buffered port:32   p_sdram_cas   = PORT_SDRAM_CAS;
out buffered port:32   p_sdram_ras   = PORT_SDRAM_RAS;
out buffered port:8    p_sdram_we    = PORT_SDRAM_WE;
out port               p_sdram_clk   = PORT_SDRAM_CLK;
clock                  sdram_cb_clk  = on SDRAM_TILE : XS1_CLKBLK_5;

/*-----------------------------------------------------------*/
/* Helpers to setup a clock as a PLL */
/*-----------------------------------------------------------*/
static void set_node_pll_reg(tileref tile_ref, unsigned reg_val){
    write_sswitch_reg(get_tile_id(tile_ref), XS1_SSWITCH_PLL_CTL_NUM, reg_val);
}

static void run_clock(void) {
    configure_clock_xcore(mclk_internal, 10); // 24.576 MHz
    configure_port_clock_output(p_mclk, mclk_internal);
    start_clock(mclk_internal);
}

static void set_pll(void) {
    set_node_pll_reg(MIC_TILE, PLL_NOM);
    run_clock();
}

void tile0_device_instantiate(
        chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend sdram_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    set_pll();
    micarray_dev_init(pdmclk, p_mclk, p_pdm_clk, p_pdm_mics);

    par {
        unsafe {
            unsafe chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, null, null};

            device_register(mic_dev_ch, eth_dev_ch, sdram_dev_ch);
            soc_peripheral_hub();
        }

        micarray_dev(
                &bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_A],
                null,
                null,
                null,
                p_pdm_mics);
    }
}

void tile1_device_instantiate(
        chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend sdram_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    par {
        eth_dev(eth_dev_ch[SOC_PERIPHERAL_TO_DMA_CH],
                eth_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH],
                eth_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                p_eth_rxclk, p_eth_rxerr, p_eth_rxd, p_eth_rxdv,
                p_eth_txclk, p_eth_txen, p_eth_txd,
                p_eth_timing, eth_rxclk, eth_txclk,
                p_smi_mdio, p_smi_mdc,
                otp_ports);

        sdram_dev(
                sdram_dev_ch[SOC_PERIPHERAL_TO_DMA_CH],
                sdram_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH],
                sdram_dev_ch[SOC_PERIPHERAL_CONTROL_CH],
                p_sdram_dq_ah,
                p_sdram_cas,
                p_sdram_ras,
                p_sdram_we,
                p_sdram_clk,
                sdram_cb_clk);
    }
}
