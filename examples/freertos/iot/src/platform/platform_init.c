// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "app_conf.h"
#include "platform/driver_instances.h"

/** TILE 0 Clock Blocks */
#define SPI_CLKBLK      XS1_CLKBLK_1
#define FLASH_CLKBLK    XS1_CLKBLK_2

static void flash_init(void)
{
#if ON_TILE(FLASH_TILE_NO)
    rtos_qspi_flash_init(
            qspi_flash_ctx,
            FLASH_CLKBLK,
            PORT_SQI_CS,
            PORT_SQI_SCLK,
            PORT_SQI_SIO,

            /** Derive QSPI clock from the 600 MHz xcore clock **/
            qspi_io_source_clock_xcore,

            /** Full speed clock configuration **/
            5, // 600 MHz / (2*5) -> 60 MHz,
            1,
            qspi_io_sample_edge_rising,
            0,

            /** SPI read clock configuration **/
            12, // 600 MHz / (2*12) -> 25 MHz
            0,
            qspi_io_sample_edge_falling,
            0,

            qspi_flash_page_program_1_4_4);
#endif
}

static void gpio_init(void)
{
#if ON_TILE(0)
    rtos_gpio_init(gpio_ctx);
#endif
}

static void spi_init(void)
{
#if ON_TILE(0)
    rtos_spi_master_init(
            spi_master_ctx,
            SPI_CLKBLK,
            WIFI_CS_N,
            WIFI_CLK,
            WIFI_MOSI,
            WIFI_MISO);

    rtos_spi_master_device_init(
            wifi_device_ctx,
            spi_master_ctx,
            1, /* WiFi CS pin is on bit 1 of the CS port */
            SPI_MODE_0,
            spi_master_source_clock_ref,
            0, /* 50 MHz */
            spi_master_sample_delay_2, /* what should this be? 2? 3? 4? */
            0, /* should this be > 0 if the above is 3-4 ? */
            1,
            0,
            0);
#endif
}

void platform_init(chanend_t other_tile_c)
{
    rtos_intertile_init(intertile_ctx, other_tile_c);

    flash_init();
    gpio_init();
    spi_init();
}
