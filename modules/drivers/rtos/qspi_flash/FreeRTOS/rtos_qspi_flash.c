// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

#include <xcore/assert.h>
#include <xcore/interrupt.h>

#include "drivers/rtos/qspi_flash/FreeRTOS/rtos_qspi_flash.h"

//typedef struct {
//    rtos_spi_master_device_t *ctx;
//    uint8_t *data_out;
//    uint8_t *data_in;
//    size_t len;
//    TaskHandle_t req_task;
//} spi_xfer_req_t;

//static void spi_xfer_thread(rtos_spi_master_t *ctx)
//{
//    spi_xfer_req_t req;
//
//    for (;;) {
//        xQueueReceive(ctx->xfer_req_queue, &req, portMAX_DELAY);
//
//        /*
//         * It would be nicer if spi_master_transfer() could handle being
//         * interrupted. At the moment it doesn't seem possible. This is
//         * the safest thing to do.
//         */
//        interrupt_mask_all();
//
//        spi_master_transfer(&req.ctx->dev_ctx,
//                req.data_out,
//                req.data_in,
//                req.len);
//
//        interrupt_unmask_all();
//
//        if (req.data_in != NULL) {
//            xTaskNotify(req.req_task, 0, eNoAction);
//        } else {
//            vPortFree(req.data_out);
//        }
//    }
//}

__attribute__((fptrgroup("rtos_qspi_flash_read_fptr_grp")))
static void qspi_flash_local_read(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;

    xSemaphoreTakeRecursive(ctx->lock, portMAX_DELAY);

    rtos_printf("Asked to read %d bytes at address 0x%08x\n", len, address);

    /*
     * Cap the address at the size of the flash.
     * This ensures the correction below will work if
     * address is outside the flash's address space.
     */
    if (address >= ctx->flash_size) {
        address = ctx->flash_size;
    }

    if (address + len > ctx->flash_size) {
        int original_len = len;

        /* Don't read past the end of the flash */
        len = ctx->flash_size - address;

        /* Return all 0xFF bytes for addresses beyond the end of the flash */
        memset(&data[len], 0xFF, original_len - len);
    }

    rtos_printf("Read %d words from flash at address 0x%x\n", len, address);

    interrupt_mask_all();
    qspi_flash_read(qspi_flash_ctx, data, address, len);
    interrupt_unmask_all();

    xSemaphoreGiveRecursive(ctx->lock);
}

static void while_busy(qspi_flash_ctx_t *ctx)
{
    uint8_t status;

    do {
        interrupt_mask_all();
        qspi_flash_read_status_register(ctx, &status, sizeof(status));
        interrupt_unmask_all();
    } while (status & QSPI_FLASH_STATUS_REG_WIP_BM);
}

__attribute__((fptrgroup("rtos_qspi_flash_write_fptr_grp")))
static void qspi_flash_local_write(
        rtos_qspi_flash_t *ctx,
        const uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;

    xSemaphoreTakeRecursive(ctx->lock, portMAX_DELAY);

    size_t bytes_left_to_write = len;
    unsigned address_to_write = address;
    const uint8_t *write_buf = data;

    rtos_printf("Asked to write %d bytes at address 0x%08x\n", bytes_left_to_write, address_to_write);

    while (bytes_left_to_write > 0) {
        /* compute the maximum number of bytes that can be written to the current page. */
        size_t max_bytes_to_write = ctx->page_size - (address_to_write & ctx->page_address_mask);
        size_t bytes_to_write = bytes_left_to_write <= max_bytes_to_write ? bytes_left_to_write : max_bytes_to_write;

        if (address_to_write >= ctx->flash_size) {
            break; /* do not write past the end of the flash */
        }

        rtos_printf("Write %d bytes from flash at address 0x%x\n", bytes_to_write, address_to_write);
        interrupt_mask_all();
        qspi_flash_write_enable(qspi_flash_ctx);
        interrupt_unmask_all();
        interrupt_mask_all();
        qspi_flash_write(qspi_flash_ctx, write_buf, address_to_write, bytes_to_write);
        interrupt_unmask_all();
        while_busy(qspi_flash_ctx);

        bytes_left_to_write -= bytes_to_write;
        write_buf += bytes_to_write;
        address_to_write += bytes_to_write;
    }

    xSemaphoreGiveRecursive(ctx->lock);
}

#define SECTORS_TO_BYTES(s, ss) ((s) * (ss))
#define BYTES_TO_SECTORS(b, ss) (((b) + (ss) - 1) / (ss))

#define SECTOR_TO_BYTE_ADDRESS(s, ss) SECTORS_TO_BYTES(s, ss)
#define BYTE_TO_SECTOR_ADDRESS(b, ss) ((b) / (ss))

#define ERASE_SIZE_4K  4096
#define ERASE_SIZE_32K 32768
#define ERASE_SIZE_64K 65536
static const int erase_sizes[] = {ERASE_SIZE_4K, ERASE_SIZE_32K, ERASE_SIZE_64K};

__attribute__((fptrgroup("rtos_qspi_flash_erase_fptr_grp")))
static void qspi_flash_local_erase(
        rtos_qspi_flash_t *ctx,
        unsigned address,
        size_t len)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;

    xSemaphoreTakeRecursive(ctx->lock, portMAX_DELAY);

    size_t bytes_left_to_erase = len;
    unsigned address_to_erase = address;

    rtos_printf("Asked to erase %d bytes at address 0x%08x\n", bytes_left_to_erase, address_to_erase);

    if (address_to_erase == 0 && bytes_left_to_erase >= ctx->flash_size) {
        /* Use chip erase when being asked to erase the entire address range */
        rtos_printf("Erasing entire chip\n");
        interrupt_mask_all();
        qspi_flash_write_enable(qspi_flash_ctx);
        interrupt_unmask_all();
        interrupt_mask_all();
        qspi_flash_erase(qspi_flash_ctx, address_to_erase, qspi_flash_erase_chip);
        interrupt_unmask_all();
        while_busy(qspi_flash_ctx);
    } else {

        if (SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(address_to_erase, erase_sizes[0]), erase_sizes[0]) != address_to_erase) {
            /*
             * If the provided starting erase address does not begin on the smallest
             * sector boundary, then update the starting address and number of bytes
             * to erase so that it does.
             */
            unsigned sector_address;
            sector_address = BYTE_TO_SECTOR_ADDRESS(address_to_erase, erase_sizes[0]);
            bytes_left_to_erase += address_to_erase - SECTOR_TO_BYTE_ADDRESS(sector_address, erase_sizes[0]);
            address_to_erase = SECTOR_TO_BYTE_ADDRESS(sector_address, erase_sizes[0]);
            rtos_printf("adjusted starting erase address to %d\n", address_to_erase);
        }

        while (bytes_left_to_erase > 0) {
            int erase_length = erase_sizes[0];
            int sector_address;
            qspi_flash_erase_length_t erase_cmd;

            if (address_to_erase >= ctx->flash_size) {
                break; /* do not erase past the end of the flash */
            }

            for (int i = 2; i > 0; i--) {
                int sector_size = erase_sizes[i];
                if (SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(address_to_erase, sector_size), sector_size) == address_to_erase) {
                    /* The address we need to erase begins on a sector boundary */
                    if (bytes_left_to_erase >= sector_size) {
                        /* And we still need to erase at least the size of this sector */
                        erase_length = sector_size;
                        break;
                    }
                }
            }

            xassert(address_to_erase == SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(address_to_erase, erase_length), erase_length));

            rtos_printf("Erasing %d bytes (%d) at byte address %d\n", erase_length, bytes_left_to_erase, address_to_erase);

            interrupt_mask_all();
            qspi_flash_write_enable(qspi_flash_ctx);
            interrupt_unmask_all();

            switch (erase_length) {
            case ERASE_SIZE_4K:
                erase_cmd = qspi_flash_erase_4k;
                break;

            case ERASE_SIZE_32K:
                erase_cmd = qspi_flash_erase_32k;
                break;

            case ERASE_SIZE_64K:
                erase_cmd = qspi_flash_erase_64k;
                break;

            default:
                xassert(0);
                bytes_left_to_erase = 0;
                continue;
            }

            interrupt_mask_all();
            qspi_flash_erase(qspi_flash_ctx, address_to_erase, erase_cmd);
            interrupt_unmask_all();

            while_busy(qspi_flash_ctx);

            address_to_erase += erase_length;
            bytes_left_to_erase -= erase_length < bytes_left_to_erase ? erase_length : bytes_left_to_erase;
        }
    }

    xSemaphoreGiveRecursive(ctx->lock);
}

#if 0
__attribute__((fptrgroup("rtos_spi_master_transfer_fptr_grp")))
static void spi_master_local_transfer(
        rtos_spi_master_device_t *ctx,
        uint8_t *data_out,
        uint8_t *data_in,
        size_t len)
{
#if 1
    interrupt_mask_all();

    spi_master_transfer(&ctx->dev_ctx,
            data_out,
            data_in,
            len);

    interrupt_unmask_all();
#else

    spi_xfer_req_t req;

    req.ctx = ctx;
    req.data_in = data_in;
    req.len = len;

    if (data_in != NULL) {
        req.data_out = data_out;
        req.req_task = xTaskGetCurrentTaskHandle();
    } else {
        /*
         * TODO: Consider a zero copy option? Caller would
         * be required to malloc data_out. Also a no-free
         * option where the caller knows that data_out will
         * still be in scope by the time the xfer is done,
         * for example if this tx only call is followed by
         * an rx.
         */
        req.data_out = pvPortMalloc(len);
        memcpy(req.data_out, data_out, len);
        req.req_task = NULL;
    }

    xQueueSend(ctx->bus_ctx->xfer_req_queue, &req, portMAX_DELAY);

    if (data_in != NULL) {
        xTaskNotifyWait(
                0x00000000UL,    /* Don't clear notification bits on entry */
                0xFFFFFFFFUL,    /* Reset full notification value on exit */
                NULL,          /* Pass out notification value into value */
                portMAX_DELAY ); /* Wait indefinitely until next notification */
    }
#endif
}
#endif

void rtos_qspi_flash_start(
        rtos_qspi_flash_t *ctx,
        unsigned priority)
{
    ctx->lock = xSemaphoreCreateRecursiveMutex();

//    ctx->xfer_req_queue = xQueueCreate(2, sizeof(spi_xfer_req_t));

//    xTaskCreate(
//                (TaskFunction_t) qspi_flash_prgm_thread,
//                "qspi_flash_prgm_thread",
//                RTOS_THREAD_STACK_SIZE(qspi_flash_prgm_thread),
//                ctx,
//                priority,
//                NULL);

    if (ctx->rpc_config != NULL && ctx->rpc_config->rpc_host_start != NULL) {
        ctx->rpc_config->rpc_host_start(ctx->rpc_config);
    }
}

void rtos_qspi_flash_init(
        rtos_qspi_flash_t *ctx,
        xclock_t clock_block,
        port_t cs_port,
        port_t sclk_port,
        port_t sio_port,
        int source_clock,
        int full_speed_clk_divisor,
        uint32_t full_speed_sclk_sample_delay,
        qspi_io_sample_edge_t full_speed_sclk_sample_edge,
        uint32_t full_speed_sio_pad_delay,
        int spi_read_clk_divisor,
        uint32_t spi_read_sclk_sample_delay,
        qspi_io_sample_edge_t spi_read_sclk_sample_edge,
        uint32_t spi_read_sio_pad_delay,
        int quad_page_program_enable,
        uint32_t quad_page_program_cmd,
        size_t page_size,
        size_t page_count)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;
    qspi_io_ctx_t *qspi_io_ctx = &qspi_flash_ctx->qspi_io_ctx;

    qspi_flash_ctx->custom_clock_setup = 0;
    qspi_flash_ctx->quad_page_program_cmd = quad_page_program_cmd;
    qspi_flash_ctx->quad_page_program_enable = quad_page_program_enable;
    qspi_flash_ctx->source_clock = source_clock;

    qspi_io_ctx->clock_block = clock_block;
    qspi_io_ctx->cs_port = cs_port;
    qspi_io_ctx->sclk_port = sclk_port;
    qspi_io_ctx->sio_port = sio_port;
    qspi_io_ctx->full_speed_clk_divisor = full_speed_clk_divisor;
    qspi_io_ctx->spi_read_clk_divisor = spi_read_clk_divisor;
    qspi_io_ctx->full_speed_sclk_sample_delay = full_speed_sclk_sample_delay;
    qspi_io_ctx->spi_read_sclk_sample_delay = spi_read_sclk_sample_delay;
    qspi_io_ctx->full_speed_sclk_sample_edge = full_speed_sclk_sample_edge;
    qspi_io_ctx->spi_read_sclk_sample_edge = spi_read_sclk_sample_edge;
    qspi_io_ctx->full_speed_sio_pad_delay = full_speed_sio_pad_delay;
    qspi_io_ctx->spi_read_sio_pad_delay = spi_read_sio_pad_delay;

    qspi_flash_init(qspi_flash_ctx);

    ctx->page_size = page_size;
    ctx->flash_size = page_size * page_count;
    ctx->page_address_mask = page_size - 1;

    /* Verify that the page size is a power of two */
    xassert((page_size != 0) && ((page_size & ctx->page_address_mask) == 0));

    ctx->rpc_config = NULL;
    ctx->read = qspi_flash_local_read;
    ctx->write = qspi_flash_local_write;
    ctx->erase = qspi_flash_local_erase;
}
