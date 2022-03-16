// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* App headers */
#include "platform_conf.h"
#include "platform/driver_instances.h"
#include "dac3101.h"

/* I2C io expander address on XCF3610_Q60A board */
#define IOEXP_I2C_ADDR        0x20

int dac3101_reg_write(uint8_t reg, uint8_t val)
{
    i2c_regop_res_t ret;

    ret = rtos_i2c_master_reg_write(i2c_master_ctx, DAC3101_I2C_DEVICE_ADDR, reg, val);

    if (ret == I2C_REGOP_SUCCESS) {
        return 0;
    } else {
        return -1;
    }
}

void dac3101_codec_reset(void)
{
    /* Set DAC_RST_N to 0 on the I2C expander (address 0x20) */
    i2c_regop_res_t ret;
    ret = rtos_i2c_master_reg_write(i2c_master_ctx, IOEXP_I2C_ADDR, 1, 0xFB);
    if (ret != I2C_REGOP_SUCCESS) {
        rtos_printf("Failed to set io expander DAC_RST_N!\n");
    }
    ret = rtos_i2c_master_reg_write(i2c_master_ctx, IOEXP_I2C_ADDR, 3, 0xFB);
    if (ret != I2C_REGOP_SUCCESS) {
        rtos_printf("Failed to set io expander DAC_RST_N!\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    ret = rtos_i2c_master_reg_write(i2c_master_ctx, IOEXP_I2C_ADDR, 1, 0xFF);
    if (ret != I2C_REGOP_SUCCESS) {
        rtos_printf("Failed to set io expander DAC_RST_N!\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

void dac3101_wait(uint32_t wait_ms)
{
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
}
