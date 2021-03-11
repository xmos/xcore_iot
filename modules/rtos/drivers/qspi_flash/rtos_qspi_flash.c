// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_QSPI_FLASH

#include <string.h>

#include <xcore/assert.h>
#include <xcore/interrupt.h>

#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"

#define FLASH_OP_READ  0
#define FLASH_OP_WRITE 1
#define FLASH_OP_ERASE 2

static void read_op(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;

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

    rtos_printf("Read %d bytes from flash at address 0x%x\n", len, address);

    interrupt_mask_all();
    qspi_flash_read(qspi_flash_ctx, data, address, len);
    interrupt_unmask_all();
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

static void write_op(
        rtos_qspi_flash_t *ctx,
        const uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;

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
}

#define SECTORS_TO_BYTES(s, ss) ((s) * (ss))
#define BYTES_TO_SECTORS(b, ss) (((b) + (ss) - 1) / (ss))

#define SECTOR_TO_BYTE_ADDRESS(s, ss) SECTORS_TO_BYTES(s, ss)
#define BYTE_TO_SECTOR_ADDRESS(b, ss) ((b) / (ss))

#define ERASE_SIZE_4K  4096
#define ERASE_SIZE_32K 32768
#define ERASE_SIZE_64K 65536
static const int erase_sizes[] = {ERASE_SIZE_4K, ERASE_SIZE_32K, ERASE_SIZE_64K};

static void erase_op(
        rtos_qspi_flash_t *ctx,
        unsigned address,
        size_t len)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;

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

    rtos_printf("Erasing complete\n");
}

typedef struct {
    int op;
    uint8_t *data;
    unsigned address;
    size_t len;
    unsigned priority;
} qspi_flash_op_req_t;

static void enable_quad_mode(rtos_qspi_flash_t *ctx)
{
    uint8_t status;

    interrupt_mask_all();
    qspi_flash_read_register(&ctx->ctx, ctx->quad_enable_register_read_cmd, &status, sizeof(status));
    interrupt_unmask_all();

    if ((status & ctx->quad_enable_bitmask) == 0) {
        status |= ctx->quad_enable_bitmask;
        interrupt_mask_all();
        qspi_flash_write_enable(&ctx->ctx);
        interrupt_unmask_all();
        interrupt_mask_all();
        qspi_flash_write_register(&ctx->ctx, ctx->quad_enable_register_write_cmd, &status, 1);
        interrupt_unmask_all();
        while_busy(&ctx->ctx);

        interrupt_mask_all();
        qspi_flash_read_register(&ctx->ctx, ctx->quad_enable_register_read_cmd, &status, sizeof(status));
        interrupt_unmask_all();
        xassert((status & ctx->quad_enable_bitmask) != 0);
    }
}

static void qspi_flash_op_thread(rtos_qspi_flash_t *ctx)
{
    qspi_flash_op_req_t op;

    /*
     * Before entering the main loop, ensure the flash is in quad mode
     */
    enable_quad_mode(ctx);

    for (;;) {
        rtos_osal_queue_receive(&ctx->op_queue, &op, RTOS_OSAL_WAIT_FOREVER);

        /*
         * Inherit the priority of the task that requested this
         * operation.
         */
        rtos_osal_thread_priority_set(&ctx->op_task, op.priority);

        switch (op.op) {
        case FLASH_OP_READ:
            read_op(ctx, op.data, op.address, op.len);
            rtos_osal_semaphore_put(&ctx->data_ready);
            break;
        case FLASH_OP_WRITE:
            write_op(ctx, op.data, op.address, op.len);
            rtos_osal_free(op.data);
            break;
        case FLASH_OP_ERASE:
            erase_op(ctx, op.address, op.len);
            break;

        }

        /*
         * Reset back to the priority set by rtos_qspi_flash_start().
         */
        rtos_osal_thread_priority_set(&ctx->op_task, ctx->op_task_priority);
    }
}

static void request(
        rtos_qspi_flash_t *ctx,
        qspi_flash_op_req_t *op)
{
    rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER);

    rtos_osal_thread_priority_get(NULL, &op->priority);
    rtos_osal_queue_send(&ctx->op_queue, op, RTOS_OSAL_WAIT_FOREVER);

    rtos_osal_mutex_put(&ctx->mutex);
}

__attribute__((fptrgroup("rtos_qspi_flash_lock_fptr_grp")))
static void qspi_flash_local_lock(
        rtos_qspi_flash_t *ctx)
{
    rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_qspi_flash_unlock_fptr_grp")))
static void qspi_flash_local_unlock(
        rtos_qspi_flash_t *ctx)
{
    rtos_osal_mutex_put(&ctx->mutex);
}

__attribute__((fptrgroup("rtos_qspi_flash_read_fptr_grp")))
static void qspi_flash_local_read(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_READ,
            .data = data,
            .address = address,
            .len = len
    };

    request(ctx, &op);

    rtos_osal_semaphore_get(&ctx->data_ready, RTOS_OSAL_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_qspi_flash_write_fptr_grp")))
static void qspi_flash_local_write(
        rtos_qspi_flash_t *ctx,
        const uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_WRITE,
            .address = address,
            .len = len
    };

    op.data = rtos_osal_malloc(len);
    memcpy(op.data, data, len);

    request(ctx, &op);
}

__attribute__((fptrgroup("rtos_qspi_flash_erase_fptr_grp")))
static void qspi_flash_local_erase(
        rtos_qspi_flash_t *ctx,
        unsigned address,
        size_t len)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_ERASE,
            .address = address,
            .len = len
    };

    request(ctx, &op);
}

void rtos_qspi_flash_start(
        rtos_qspi_flash_t *ctx,
        unsigned priority)
{
    rtos_osal_mutex_create(&ctx->mutex, "qspi_lock", RTOS_OSAL_RECURSIVE);
    rtos_osal_queue_create(&ctx->op_queue, "qspi_req_queue", 2, sizeof(qspi_flash_op_req_t));
    rtos_osal_semaphore_create(&ctx->data_ready, "qspi_dr_sem", 1, 0);

    ctx->op_task_priority = priority;
    rtos_osal_thread_create(
            &ctx->op_task,
            "qspi_flash_op_thread",
            (rtos_osal_entry_function_t) qspi_flash_op_thread,
            ctx,
            RTOS_THREAD_STACK_SIZE(qspi_flash_op_thread),
            priority);

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
        qspi_io_source_clock_t source_clock,
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
        uint32_t quad_enable_register_read_cmd,
        uint32_t quad_enable_register_write_cmd,
        uint32_t quad_enable_bitmask,
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

    ctx->quad_enable_register_read_cmd = quad_enable_register_read_cmd;
    ctx->quad_enable_register_write_cmd = quad_enable_register_write_cmd;
    ctx->quad_enable_bitmask = quad_enable_bitmask;
    ctx->page_size = page_size;
    ctx->flash_size = page_size * page_count;
    ctx->page_address_mask = page_size - 1;

    /* Verify that the page size is a power of two */
    xassert((page_size != 0) && ((page_size & ctx->page_address_mask) == 0));

    ctx->rpc_config = NULL;
    ctx->read = qspi_flash_local_read;
    ctx->write = qspi_flash_local_write;
    ctx->erase = qspi_flash_local_erase;
    ctx->lock = qspi_flash_local_lock;
    ctx->unlock = qspi_flash_local_unlock;
}
