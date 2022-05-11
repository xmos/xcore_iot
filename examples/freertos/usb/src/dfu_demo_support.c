// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* App headers */
#include "rtos_usb.h"
#include "usb_support.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

#if DFU_DEMO
#define QSPI_FLASH_SECTOR_SIZE 4096
#define MODE_ADDR   0x200000
static int mode = 0;
void write_dfu_mode(void);

int check_dfu_mode(void)
{
    rtos_qspi_flash_read(
        qspi_flash_ctx,
        (uint8_t*)&mode,
        (unsigned)(MODE_ADDR),
        (size_t)sizeof(int));
        // rtos_printf("Mode is %u\n", mode);
    if(mode == 0xffffffff) mode = 0;    // uninitialized should be handled as RT
    return mode;
}

void set_rt_mode(void)
{
    mode = 0;
    write_dfu_mode();
}

void set_dfu_mode(void)
{
    mode = 1;
    write_dfu_mode();
}

void write_dfu_mode(void)
{
    uint8_t *tmp_buf = rtos_osal_malloc( sizeof(uint8_t) * QSPI_FLASH_SECTOR_SIZE);
    rtos_qspi_flash_lock(qspi_flash_ctx);
    {
        rtos_qspi_flash_read(
                qspi_flash_ctx,
                tmp_buf,
                (unsigned)(MODE_ADDR),
                (size_t)QSPI_FLASH_SECTOR_SIZE);

        memcpy(tmp_buf, &mode, sizeof(int));

        rtos_qspi_flash_erase(
                qspi_flash_ctx,
                (unsigned)(MODE_ADDR),
                (size_t)QSPI_FLASH_SECTOR_SIZE);
        rtos_qspi_flash_write(
                qspi_flash_ctx,
                (uint8_t *) tmp_buf,
                (unsigned)(MODE_ADDR),
                (size_t)QSPI_FLASH_SECTOR_SIZE);
    }
    rtos_qspi_flash_unlock(qspi_flash_ctx);

    rtos_osal_free(tmp_buf);
}

size_t boot_image_read(void* ctx, unsigned addr, uint8_t *buf, size_t len)
{
    rtos_qspi_flash_t *qspi = (rtos_qspi_flash_t *)ctx;
    rtos_qspi_flash_read(qspi, buf, addr, len);
    return len;
}

size_t boot_image_write(void* ctx, unsigned addr, const uint8_t *buf, size_t len)
{
    rtos_qspi_flash_t *qspi = (rtos_qspi_flash_t *)ctx;

    uint8_t *tmp_buf = rtos_osal_malloc( sizeof(uint8_t) * QSPI_FLASH_SECTOR_SIZE);
    rtos_qspi_flash_lock(qspi);
    {
        rtos_qspi_flash_read(
                qspi,
                tmp_buf,
                (unsigned)(addr),
                (size_t)QSPI_FLASH_SECTOR_SIZE);

        memcpy(tmp_buf, buf, len);

        rtos_qspi_flash_erase(
                qspi,
                (unsigned)(addr),
                (size_t)QSPI_FLASH_SECTOR_SIZE);
        rtos_qspi_flash_write(
                qspi,
                (uint8_t *) tmp_buf,
                (unsigned)(addr),
                (size_t)QSPI_FLASH_SECTOR_SIZE);
    }
    rtos_qspi_flash_unlock(qspi);

    rtos_osal_free(tmp_buf);

    return len;
}
#endif
