// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_QSPI_FLASH

#include <string.h>

#include <xcore/assert.h>
#include <xcore/interrupt.h>
#include <xcore/lock.h>

#include "rtos_qspi_flash.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define FLASH_OP_READ  0
#define FLASH_OP_WRITE 1
#define FLASH_OP_ERASE 2

extern unsigned __libc_hwlock;

/*
 * Returns true if the spinlock is
 * acquired, false if not available.
 * NOT recursive.
 */
static bool spinlock_get(volatile int *lock)
{
    bool ret;

    lock_acquire(__libc_hwlock);
    {
        if (*lock == 0) {
            *lock = 1;
            ret = true;
        } else {
            ret = false;
        }
    }
    lock_release(__libc_hwlock);

    return ret;
}

/*
 * Releases the lock. It MUST be owned
 * by the caller.
 */
static void spinlock_release(volatile int *lock)
{
    *lock = 0;
}

int rtos_qspi_flash_read_ll(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;
    uint32_t irq_mask;
    bool lock_acquired;

    rtos_printf("Asked to ll read %d bytes at address 0x%08x\n", len, address);

    irq_mask = rtos_interrupt_mask_all();
    lock_acquired = spinlock_get(&ctx->spinlock);

    while (lock_acquired && len > 0) {

        size_t read_len = MIN(len, RTOS_QSPI_FLASH_READ_CHUNK_SIZE);

        /*
         * Cap the address at the size of the flash.
         * This ensures the correction below will work if
         * address is outside the flash's address space.
         */
        if (address >= ctx->flash_size) {
            address = ctx->flash_size;
        }

        if (address + read_len > ctx->flash_size) {
            int original_len = read_len;

            /* Don't read past the end of the flash */
            read_len = ctx->flash_size - address;

            /* Return all 0xFF bytes for addresses beyond the end of the flash */
            memset(&data[read_len], 0xFF, original_len - read_len);
        }

        rtos_printf("Read %d bytes from flash at address 0x%x\n", read_len, address);
        qspi_flash_read(qspi_flash_ctx, data, address, read_len);

        len -= read_len;
        data += read_len;
        address += read_len;
    }

    if (lock_acquired) {
        spinlock_release(&ctx->spinlock);
    }
    rtos_interrupt_mask_set(irq_mask);

    return lock_acquired ? 0 : -1;
}

static void read_op(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;

    rtos_printf("Asked to read %d bytes at address 0x%08x\n", len, address);

    while (len > 0) {

        size_t read_len = MIN(len, RTOS_QSPI_FLASH_READ_CHUNK_SIZE);

        /*
         * Cap the address at the size of the flash.
         * This ensures the correction below will work if
         * address is outside the flash's address space.
         */
        if (address >= ctx->flash_size) {
            address = ctx->flash_size;
        }

        if (address + read_len > ctx->flash_size) {
            int original_len = read_len;

            /* Don't read past the end of the flash */
            read_len = ctx->flash_size - address;

            /* Return all 0xFF bytes for addresses beyond the end of the flash */
            memset(&data[read_len], 0xFF, original_len - read_len);
        }

        rtos_printf("Read %d bytes from flash at address 0x%x\n", read_len, address);

        interrupt_mask_all();
        qspi_flash_read(qspi_flash_ctx, data, address, read_len);
        interrupt_unmask_all();

        len -= read_len;
        data += read_len;
        address += read_len;
    }
}

static void while_busy(qspi_flash_ctx_t *ctx)
{
    bool busy;

    do {
        interrupt_mask_all();
        busy = qspi_flash_write_in_progress(ctx);
        interrupt_unmask_all();
    } while (busy);
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
        size_t max_bytes_to_write = qspi_flash_ctx->page_size_bytes - (address_to_write & (qspi_flash_ctx->page_size_bytes - 1));
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

#define SECTORS_TO_BYTES(s, ss_log2) ((s) << (ss_log2))
#define BYTES_TO_SECTORS(b, ss_log2) (((b) + (1 << ss_log2) - 1) >> (ss_log2))

#define SECTOR_TO_BYTE_ADDRESS(s, ss_log2) SECTORS_TO_BYTES(s, ss_log2)
#define BYTE_TO_SECTOR_ADDRESS(b, ss_log2) ((b) >> (ss_log2))

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

        if (SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(address_to_erase, qspi_flash_erase_type_size_log2(qspi_flash_ctx, 0)), qspi_flash_erase_type_size_log2(qspi_flash_ctx, 0)) != address_to_erase) {
            /*
             * If the provided starting erase address does not begin on the smallest
             * sector boundary, then update the starting address and number of bytes
             * to erase so that it does.
             */
            unsigned sector_address;
            sector_address = BYTE_TO_SECTOR_ADDRESS(address_to_erase, qspi_flash_erase_type_size_log2(qspi_flash_ctx, 0));
            bytes_left_to_erase += address_to_erase - SECTOR_TO_BYTE_ADDRESS(sector_address, qspi_flash_erase_type_size_log2(qspi_flash_ctx, 0));
            address_to_erase = SECTOR_TO_BYTE_ADDRESS(sector_address, qspi_flash_erase_type_size_log2(qspi_flash_ctx, 0));
            rtos_printf("adjusted starting erase address to %d\n", address_to_erase);
        }

        while (bytes_left_to_erase > 0) {
            int erase_length;
            int erase_length_log2 = qspi_flash_erase_type_size_log2(qspi_flash_ctx, 0);
            qspi_flash_erase_length_t erase_cmd = qspi_flash_erase_1;

            if (address_to_erase >= ctx->flash_size) {
                break; /* do not erase past the end of the flash */
            }

            for (qspi_flash_erase_length_t i = qspi_flash_erase_4; i > qspi_flash_erase_1; i--) {
                int sector_size_log2 = qspi_flash_erase_type_size_log2(qspi_flash_ctx, i);
                if (sector_size_log2 != 0 && SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(address_to_erase, sector_size_log2), sector_size_log2) == address_to_erase) {
                    /* The address we need to erase begins on a sector boundary */
                    if (bytes_left_to_erase >= (1 << sector_size_log2)) {
                        /* And we still need to erase at least the size of this sector */
                        erase_length_log2 = sector_size_log2;
                        erase_cmd = i;
                        break;
                    }
                }
            }

            erase_length = 1 << erase_length_log2;

            xassert(address_to_erase == SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(address_to_erase, erase_length_log2), erase_length_log2));

            rtos_printf("Erasing %d bytes (%d) at byte address %d\n", erase_length, bytes_left_to_erase, address_to_erase);

            interrupt_mask_all();
            qspi_flash_write_enable(qspi_flash_ctx);
            interrupt_unmask_all();

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

static void qspi_flash_op_thread(rtos_qspi_flash_t *ctx)
{
    qspi_flash_op_req_t op;
    bool quad_enabled;

    quad_enabled = qspi_flash_quad_enable_write(&ctx->ctx, true);
    xassert(quad_enabled && "QE bit could not be set\n");

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
    bool mutex_owned = (xSemaphoreGetMutexHolder(ctx->mutex.mutex) == xTaskGetCurrentTaskHandle());

    while (rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER) != RTOS_OSAL_SUCCESS);

    if (!mutex_owned) {
        while (!spinlock_get(&ctx->spinlock));
    } else {
        /*
         * The spinlock is already owned by this thread, so safe
         * to just increment it to keep track of the recursion.
         */
        ctx->spinlock++;
    }
}

__attribute__((fptrgroup("rtos_qspi_flash_unlock_fptr_grp")))
static void qspi_flash_local_unlock(
        rtos_qspi_flash_t *ctx)
{
    bool mutex_owned = (xSemaphoreGetMutexHolder(ctx->mutex.mutex) == xTaskGetCurrentTaskHandle());

    if (mutex_owned) {
        /*
         * Since the spinlock is already owned by this thread, it is safe
         * to just decrement it to unwind any recursion. Once it is
         * decremented down to 0 it will be released. Other threads will
         * not be able to acquire it until the mutex is also released, but
         * it will be possible a call to rtos_qspi_flash_read_ll() to
         * immediately acquire it.
         */
        ctx->spinlock--;
        rtos_osal_mutex_put(&ctx->mutex);
    }
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
        qspi_flash_page_program_cmd_t quad_page_program_cmd)
{
    qspi_flash_ctx_t *qspi_flash_ctx = &ctx->ctx;
    qspi_io_ctx_t *qspi_io_ctx = &qspi_flash_ctx->qspi_io_ctx;

    qspi_flash_ctx->custom_clock_setup = 1;
    qspi_flash_ctx->quad_page_program_cmd = quad_page_program_cmd;
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

    ctx->flash_size = qspi_flash_ctx->flash_size_kbytes * 1024;
    xassert(ctx->flash_size > qspi_flash_ctx->flash_size_kbytes);

    /* Verify that the page size is a power of two */
    xassert((qspi_flash_ctx->page_size_bytes != 0) && ((qspi_flash_ctx->page_size_bytes & (qspi_flash_ctx->page_size_bytes - 1)) == 0));

    ctx->rpc_config = NULL;
    ctx->read = qspi_flash_local_read;
    ctx->write = qspi_flash_local_write;
    ctx->erase = qspi_flash_local_erase;
    ctx->lock = qspi_flash_local_lock;
    ctx->unlock = qspi_flash_local_unlock;
}
