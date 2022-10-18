// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "app_conf.h"
#include "board_init.h"

static rtos_driver_rpc_t spi_master_rpc_config;

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_spi_master_device_t *test_device_ctx,
        rtos_uart_tx_t *rtos_uart_tx_ctx
    )
{
    rtos_intertile_init(intertile_ctx, tile1);
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

    rtos_spi_master_init(
            spi_master_ctx,
            XS1_CLKBLK_1,
            XS1_PORT_1O,    /* CS PORT_I2C_SDA */
            XS1_PORT_1H,    /* SCLK */
            XS1_PORT_1E,    /* MOSI */
            XS1_PORT_1F);   /* MISO */

    rtos_spi_master_device_init(
            test_device_ctx,
            spi_master_ctx,
            0, /* CS pin is on bit 0 of the CS port */
            SPI_MODE_0,
            spi_master_source_clock_ref,
            1, /* 50 MHz */
            spi_master_sample_delay_2,
            0,
            10000,
            10000,
            10000);

    rtos_spi_master_device_t *spi_devices_ctx[1] = {test_device_ctx};

    rtos_spi_master_rpc_host_init(
            spi_master_ctx,
            spi_devices_ctx,
            1,
            &spi_master_rpc_config,
            client_intertile_ctx,
            1);

    hwtimer_t tmr_tx = hwtimer_alloc();

    rtos_uart_tx_init(
            rtos_uart_tx_ctx,
            XS1_PORT_1N,
            UART_BAUD_RATE,
            8,
            UART_PARITY_ODD,
            1,
            tmr_tx);

}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_spi_master_device_t *test_device_ctx,
        rtos_spi_slave_t *spi_slave_ctx,
        rtos_uart_rx_t *rtos_uart_rx_ctx
    )
{
    rtos_intertile_init(intertile_ctx, tile0);

    rtos_spi_slave_init(
            spi_slave_ctx,
            SPI_SLAVE_CORE_MASK,
            XS1_CLKBLK_2,
            SPI_TEST_CPOL,
            SPI_TEST_CPHA,
            XS1_PORT_1B,    /* SCLK PORT_I2S_LRCLK */
            XS1_PORT_1A,    /* MOSI PORT_I2S_DAC_DATA */
            XS1_PORT_1N,    /* MISO PORT_I2S_ADC_DATA */
            XS1_PORT_1O);   /* CS */

    rtos_spi_master_device_t *spi_devices_ctx[1] = {test_device_ctx};

    rtos_spi_master_rpc_client_init(
            spi_master_ctx,
            spi_devices_ctx,
            1,
            &spi_master_rpc_config,
            intertile_ctx);

    hwtimer_t tmr_rx = hwtimer_alloc();

    rtos_uart_rx_init(
            rtos_uart_rx_ctx,
            UART_RX_CORE_MASK,
            XS1_PORT_1M ,
            UART_BAUD_RATE,
            8,
            UART_PARITY_ODD,
            1,
            tmr_rx);
}
