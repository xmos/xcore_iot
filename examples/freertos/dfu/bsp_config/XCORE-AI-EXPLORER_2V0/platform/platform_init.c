// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>
#include <quadflashlib.h>

#include "platform_conf.h"
#include "platform/driver_instances.h"
#include <xcore/clock.h>
#include <xcore/port.h>

static void flash_init(void)
{
#if ON_TILE(FLASH_TILE_NO)
    fl_QuadDeviceSpec qspi_spec = BOARD_QSPI_SPEC;
    fl_QSPIPorts qspi_ports = {
        .qspiCS = PORT_SQI_CS,
        .qspiSCLK = PORT_SQI_SCLK,
        .qspiSIO = PORT_SQI_SIO,
        .qspiClkblk = FLASH_CLKBLK,
    };

    rtos_dfu_image_init(
            dfu_image_ctx,
            &qspi_ports,
            &qspi_spec,
            1);

    qspi_flash_ctx->ctx.sfdp_skip = true;
    qspi_flash_ctx->ctx.sfdp_supported = false;
    qspi_flash_ctx->ctx.page_size_bytes = 256;
    qspi_flash_ctx->ctx.page_count = 16384;
    qspi_flash_ctx->ctx.flash_size_kbytes = 4096;
    qspi_flash_ctx->ctx.address_bytes = 3;
    qspi_flash_ctx->ctx.erase_info[0].size_log2 = 12;
    qspi_flash_ctx->ctx.erase_info[0].cmd = 0xEEFEEEEE;
    qspi_flash_ctx->ctx.erase_info[1].size_log2 = 15;
    qspi_flash_ctx->ctx.erase_info[1].cmd = 0xEFEFEEFE;
    qspi_flash_ctx->ctx.erase_info[2].size_log2 = 16;
    qspi_flash_ctx->ctx.erase_info[2].cmd = 0xFFEFFEEE;
    qspi_flash_ctx->ctx.erase_info[3].size_log2 = 0;
    qspi_flash_ctx->ctx.erase_info[3].cmd = 0;
    qspi_flash_ctx->ctx.busy_poll_cmd = 0xEEEEEFEF;
    qspi_flash_ctx->ctx.busy_poll_bit = 0;
    qspi_flash_ctx->ctx.busy_poll_ready_value = 0;
    qspi_flash_ctx->ctx.qe_reg = 2;
    qspi_flash_ctx->ctx.qe_bit = 1;
    qspi_flash_ctx->ctx.sr2_read_cmd = 0xEEFFEFEF;
    qspi_flash_ctx->ctx.sr2_write_cmd = 0xEEEEEEEE;

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

            qspi_flash_page_program_1_1_4);
#endif
}

static void gpio_init(void)
{
    static rtos_driver_rpc_t gpio_rpc_config_t0;
    static rtos_driver_rpc_t gpio_rpc_config_t1;
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

#if ON_TILE(0)
    rtos_gpio_init(gpio_ctx_t0);

    rtos_gpio_rpc_host_init(
            gpio_ctx_t0,
            &gpio_rpc_config_t0,
            client_intertile_ctx,
            1);

    rtos_gpio_rpc_client_init(
            gpio_ctx_t1,
            &gpio_rpc_config_t1,
            intertile_ctx);
#endif

#if ON_TILE(1)
    rtos_gpio_init(gpio_ctx_t1);

    rtos_gpio_rpc_client_init(
            gpio_ctx_t0,
            &gpio_rpc_config_t0,
            intertile_ctx);

    rtos_gpio_rpc_host_init(
            gpio_ctx_t1,
            &gpio_rpc_config_t1,
            client_intertile_ctx,
            1);
#endif
}

static void i2c_init(void)
{
    static rtos_driver_rpc_t i2c_rpc_config;

#if ON_TILE(I2C_TILE_NO)
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};
    rtos_i2c_master_init(
            i2c_master_ctx,
            PORT_I2C_SCL, 0, 0,
            PORT_I2C_SDA, 0, 0,
            0,
            100);

    rtos_i2c_master_rpc_host_init(
            i2c_master_ctx,
            &i2c_rpc_config,
            client_intertile_ctx,
            1);
#else
    rtos_i2c_master_rpc_client_init(
            i2c_master_ctx,
            &i2c_rpc_config,
            intertile_ctx);
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


static void uart_init(void)
{
#if ON_TILE(UART_TILE_NO)
    hwtimer_t tmr_rx = hwtimer_alloc();

    rtos_uart_rx_init(
            uart_rx_ctx,
            (1 << appconfUART_RX_IO_CORE),
            XS1_PORT_1M, //X1D36
            appconfUART_BAUD_RATE,
            8,
            UART_PARITY_NONE,
            1,
            tmr_rx);


    hwtimer_t tmr_tx = hwtimer_alloc();

    rtos_uart_tx_init(
            uart_tx_ctx,
            XS1_PORT_1P,  //X1D39
            appconfUART_BAUD_RATE,
            8,
            UART_PARITY_NONE,
            1,
            tmr_tx);
#endif
}

void usb_init(void)
{
#if ON_TILE(USB_TILE_NO)
    usb_manager_init();
#endif
}

void platform_init(chanend_t other_tile_c)
{
    rtos_intertile_init(intertile_ctx, other_tile_c);

    flash_init();
    gpio_init();
    spi_init();
    i2c_init();
    uart_init();
    usb_init();
}
