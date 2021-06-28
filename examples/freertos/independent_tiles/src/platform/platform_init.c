// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "app_conf.h"
#include "driver_instances.h"
#include "platform_init.h"

typedef enum {
  PORT_INPUT = 0,
  PORT_OUTPUT = 1,
  PORT_INOUT = 2,
} port_direction_t;

/* clocks and ports */
xclock_t pdmclk;
xclock_t pdmclk2;
xclock_t bclk;
port_t p_mclk;
port_t p_pdm_clk;
port_t p_pdm_mics;
port_t p_i2s_dout[1] = {PORT_I2S_DAC_DATA};
port_t p_i2s_din[1] = {PORT_I2S_ADC_DATA};
port_t p_bclk = PORT_I2S_BCLK;
port_t p_lrclk = PORT_I2S_BCLK;

static port_t port_init(port_t p, port_direction_t d, port_type_t t, ...) {
  if (t == PORT_UNBUFFERED) {
    port_enable(p);
  } else {
    size_t tw;
    va_list ap;
    va_start(ap, t);
    tw = va_arg(ap, size_t);
    va_end(ap);
    port_start_buffered(p, tw);
  }

  if (d == PORT_OUTPUT) {
    /* ensure port is in output mode. */
    port_out(p, 0);
  }

  return p;
}

static xclock_t clock_init(xclock_t c) {
  clock_enable(c);

  return c;
}

static void set_app_pll(void) {
  unsigned tileid = get_local_tile_id();

  xassert(appconfAUDIO_CLOCK_FREQUENCY == 24576000);

  // 24MHz in, 24.576MHz out, integer mode
  // Found exact solution:   IN  24000000.0, OUT  24576000.0, VCO 2457600000.0,
  // RD  5, FD  512                       , OD  5, FOD   10
  const unsigned APP_PLL_DISABLE = 0x0201FF04;
  const unsigned APP_PLL_CTL_0 = 0x0A01FF04;
  const unsigned APP_PLL_DIV_0 = 0x80000004;
  const unsigned APP_PLL_FRAC_0 = 0x00000000;

  write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, APP_PLL_DISABLE);

  hwtimer_t tmr = hwtimer_alloc();
  {
    xassert(tmr != 0);
    hwtimer_delay(tmr, 100000); // 1ms with 100 MHz timer tick
  }
  hwtimer_free(tmr);

  write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, APP_PLL_CTL_0);
  write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, APP_PLL_CTL_0);
  write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_PLL_FRAC_N_DIVIDER_NUM,
                    APP_PLL_FRAC_0);
  write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_CLK_DIVIDER_NUM, APP_PLL_DIV_0);
}

#if appconfI2C_RPC_ENABLED
static rtos_driver_rpc_t i2c_rpc_config;
#endif
#if appconfMIC_ARRAY_RPC_ENABLED
static rtos_driver_rpc_t mic_rpc_config;
#endif
#if appconfI2S_RPC_ENABLED
static rtos_driver_rpc_t i2s_rpc_config;
#endif
#if appconfGPIO_RPC_ENABLED
static rtos_driver_rpc_t gpio_rpc_config;
#endif
#if appconfSPI_RPC_ENABLED
static rtos_driver_rpc_t spi_rpc_config;
#endif
#if appconfQSPI_FLASH_RPC_ENABLED
static rtos_driver_rpc_t qspi_flash_rpc_config;
#endif

void mclk_port_init() {
#if ON_TILE(1)
  {
    port_t p_rst_shared =
        port_init(PORT_CODEC_RST_N, PORT_OUTPUT, PORT_UNBUFFERED);
    port_out(p_rst_shared, 0xF);

    /* Clock blocks for PDM mics */
    pdmclk = clock_init(XS1_CLKBLK_1);
    pdmclk2 = clock_init(XS1_CLKBLK_2);

    /* Clock port for the PDM mics and I2S */
    p_mclk = port_init(PORT_MCLK_IN, PORT_INPUT, PORT_UNBUFFERED);

    /* Ports for the PDM microphones */
    p_pdm_clk = port_init(PORT_PDM_CLK, PORT_OUTPUT, PORT_UNBUFFERED);
    p_pdm_mics = port_init(PORT_PDM_DATA, PORT_INPUT, PORT_BUFFERED, 32);

    /* Clock blocks for I2S */
    bclk = clock_init(XS1_CLKBLK_3);

    set_app_pll();
  }
#endif
}

void gpio_init() {
#if appconfGPIO_RPC_ENABLED
#if ON_TILE(0)
  {
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};
    rtos_gpio_init(gpio_ctx);
    rtos_gpio_rpc_host_init(gpio_ctx, &gpio_rpc_config, client_intertile_ctx,
                            1);
  }
#else
  { rtos_gpio_rpc_client_init(gpio_ctx, &gpio_rpc_config, intertile1_ctx); }
#endif
#endif
}

void i2c_init() {
#if appconfI2C_RPC_ENABLED
#if ON_TILE(0)
  {
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};
    rtos_i2c_master_rpc_host_init(i2c_master_ctx, &i2c_rpc_config,
                                  client_intertile_ctx, 1);
  }
#else
  {
    rtos_i2c_master_rpc_client_init(i2c_master_ctx, &i2c_rpc_config,
                                    intertile1_ctx);
  }
#endif
#endif
}

void spi_init() {
#if appconfSPI_RPC_ENABLED
#if ON_TILE(0)
  {
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};
    rtos_spi_master_init(spi_master_ctx, XS1_CLKBLK_1, WIFI_CS_N, WIFI_CLK,
                         WIFI_MOSI, WIFI_MISO);

    rtos_spi_master_device_init(
        wifi_device_ctx, spi_master_ctx,
        1, /* WiFi CS pin is on bit 1 of the CS port */
        SPI_MODE_0, spi_master_source_clock_ref, 0, /* 50 MHz */
        spi_master_sample_delay_2, /* what should this be? 2? 3? 4? */
        0,                         /* should this be > 0 if the above is 3-4 ?
                                    */
        1, 0, 0);

    rtos_spi_master_rpc_host_init(spi_master_ctx, &wifi_device_ctx, 1,
                                  &spi_rpc_config, client_intertile_ctx, 1);
  }
#else
  {
    rtos_spi_master_rpc_client_init(spi_master_ctx, &wifi_device_ctx, 1,
                                    &spi_rpc_config, intertile1_ctx);
  }
#endif
#endif
}

void i2s_init() {
#if appconfI2S_RPC_ENABLED
#if ON_TILE(1)
  {
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};
    rtos_i2s_master_init(i2s_ctx, ~(1 << 0), p_i2s_dout, 1, p_i2s_din,
                         appconfI2S_ADC_ENABLED ? 1 : 0, p_bclk, p_lrclk,
                         p_mclk, bclk);

    rtos_i2s_rpc_host_init(i2s_ctx, &i2s_rpc_config, client_intertile_ctx, 1);
  }
#else
  { rtos_i2s_rpc_client_init(i2s_ctx, &i2s_rpc_config, intertile1_ctx); }
#endif
#endif
}

void mics_init() {
#if appconfMIC_ARRAY_RPC_ENABLED
#if ON_TILE(1)
  {
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};
    rtos_mic_array_init(mic_array_ctx, ~(1 << 0), pdmclk, pdmclk2,
                        appconfAUDIO_CLOCK_FREQUENCY /
                            appconfPDM_CLOCK_FREQUENCY,
                        p_mclk, p_pdm_clk, p_pdm_mics);

    rtos_mic_array_rpc_host_init(mic_array_ctx, &mic_rpc_config,
                                 client_intertile_ctx, 1);
  }
#else
  {
    rtos_mic_array_rpc_client_init(mic_array_ctx, &mic_rpc_config,
                                   intertile1_ctx);
  }
#endif
#endif
}

void flash_init() {
#if appconfQSPI_FLASH_RPC_ENABLED
#if ON_TILE(0)
  {
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};
    rtos_qspi_flash_init(qspi_flash_ctx, XS1_CLKBLK_2, PORT_SQI_CS,
                         PORT_SQI_SCLK, PORT_SQI_SIO,

                         /** Derive QSPI clock from the 600 MHz xcore clock **/
                         qspi_io_source_clock_xcore,

                         /** Full speed clock configuration **/
                         5, // 600 MHz / (2*5) -> 60 MHz,
                         1, qspi_io_sample_edge_rising, 0,

                         /** SPI read clock configuration **/
                         12, // 600 MHz / (2*12) -> 25 MHz
                         0, qspi_io_sample_edge_falling, 0,

                         qspi_flash_page_program_1_4_4);

    rtos_qspi_flash_rpc_host_init(qspi_flash_ctx, &qspi_flash_rpc_config,
                                  client_intertile_ctx, 1);
  }
#else
  {
    rtos_qspi_flash_rpc_client_init(qspi_flash_ctx, &qspi_flash_rpc_config,
                                    intertile1_ctx);
  }
#endif
#endif
}

void platform_init(chanend_t other_tile_c) {
  mclk_port_init();

  rtos_intertile_init(intertile1_ctx, other_tile_c);
  rtos_intertile_init(intertile2_ctx, other_tile_c);
  i2c_init();
  spi_init();
  flash_init();
  gpio_init();
  i2s_init();
  mics_init();
}