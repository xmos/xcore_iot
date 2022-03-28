// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_QSPI_FLASH_H_
#define RTOS_QSPI_FLASH_H_

/**
 * \addtogroup rtos_qspi_flash_driver rtos_qspi_flash_driver
 *
 * The public API for using the RTOS QSPI flash driver.
 * @{
 */

#include "qspi_flash.h"

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"

#define RTOS_QSPI_FLASH_READ_CHUNK_SIZE (24*1024)

/**
 * Typedef to the RTOS QSPI flash driver instance struct.
 */
typedef struct rtos_qspi_flash_struct rtos_qspi_flash_t;

/**
 * Struct representing an RTOS QSPI flash driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_qspi_flash_struct {
    rtos_driver_rpc_t *rpc_config;

    __attribute__((fptrgroup("rtos_qspi_flash_read_fptr_grp")))
    void (*read)(rtos_qspi_flash_t *, uint8_t *, unsigned, size_t);

    __attribute__((fptrgroup("rtos_qspi_flash_write_fptr_grp")))
    void (*write)(rtos_qspi_flash_t *, const uint8_t *, unsigned, size_t);

    __attribute__((fptrgroup("rtos_qspi_flash_erase_fptr_grp")))
    void (*erase)(rtos_qspi_flash_t *, unsigned, size_t);

    __attribute__((fptrgroup("rtos_qspi_flash_lock_fptr_grp")))
    void (*lock)(rtos_qspi_flash_t *);

    __attribute__((fptrgroup("rtos_qspi_flash_unlock_fptr_grp")))
    void (*unlock)(rtos_qspi_flash_t *);

    qspi_flash_ctx_t ctx;
    size_t flash_size;

    unsigned op_task_priority;
    rtos_osal_thread_t op_task;
    rtos_osal_queue_t op_queue;
    rtos_osal_semaphore_t data_ready;
    rtos_osal_mutex_t mutex;
    volatile int spinlock;
};

#include "rtos_qspi_flash_rpc.h"

/**
 * \addtogroup rtos_qspi_flash_driver_core rtos_qspi_flash_driver_core
 *
 * The core functions for using an RTOS QSPI flash driver instance after
 * it has been initialized and started. These functions may be used
 * by both the host and any client tiles that RPC has been enabled for.
 * @{
 */

/**
 * Obtains a lock for exclusive access to the QSPI flash. This allows
 * a thread to perform a sequence of operations (such as read, modify, erase,
 * write) without the risk of another thread issuing a command in the middle of
 * the sequence and corrupting the data in the flash.
 *
 * If only a single atomic operation needs to be performed, such as a read, it
 * is not necessary to call this to obtain the lock first. Each individual operation
 * obtains and releases the lock automatically so that they cannot run while another
 * thread has the lock.
 *
 * The lock MUST be released when it is no longer needed by calling
 * rtos_qspi_flash_unlock().
 *
 * \param ctx  A pointer to the QSPI flash driver instance to lock.
 */
inline void rtos_qspi_flash_lock(
        rtos_qspi_flash_t *ctx)
{
    ctx->lock(ctx);
}

/**
 * Releases a lock for exclusive access to the QSPI flash. The lock
 * must have already been obtained by calling rtos_qspi_flash_lock().
 *
 * \param ctx  A pointer to the QSPI flash driver instance to unlock.
 */
inline void rtos_qspi_flash_unlock(
        rtos_qspi_flash_t *ctx)
{
    ctx->unlock(ctx);
}

/**
 * This reads data from the flash in quad I/O mode. All four lines are
 * used to send the address and to read the data.
 *
 * \param ctx     A pointer to the QSPI flash driver instance to use.
 * \param data    Pointer to the buffer to save the read data to.
 * \param address The byte address in the flash to begin reading at.
 *                Only bits 23:0 contain the address. Bits 31:24 are actually
 *                transmitted to the flash during the first two dummy cycles
 *                following the three address bytes. Some flashes read the SIO
 *                lines during these first two dummy cycles to enable certain
 *                features, so this might be useful for some applications.
 * \param len     The number of bytes to read and save to \p data.
 */
inline void rtos_qspi_flash_read(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    ctx->read(ctx, data, address, len);
}

/**
 * This is a lower level version of rtos_qspi_flash_read() that is safe
 * to call from within ISRs. If a task currently own the flash lock, or
 * if another core is actively doing a read with this function, then the
 * read will not be performed and an error returned. It is up to the
 * application to determine what it should do in this situation and to
 * avoid a potential deadlock.
 *
 * \note It is not possible to call this from a task that currently owns
 * the flash lock taken with rtos_qspi_flash_lock(). In general it is not
 * advisable to call this from an RTOS task unless the small amount of
 * overhead time that is introduced by rtos_qspi_flash_read() is unacceptable.
 *
 * \param ctx     A pointer to the QSPI flash driver instance to use.
 * \param data    Pointer to the buffer to save the read data to.
 * \param address The byte address in the flash to begin reading at.
 *                Only bits 23:0 contain the address. Bits 31:24 are actually
 *                transmitted to the flash during the first two dummy cycles
 *                following the three address bytes. Some flashes read the SIO
 *                lines during these first two dummy cycles to enable certain
 *                features, so this might be useful for some applications.
 * \param len     The number of bytes to read and save to \p data.
 *
 * \retval        0 if the flash was available and the read operation was performed.
 * \retval       -1 if the flash was unavailable and the read could not be performed.
 */
int rtos_qspi_flash_read_ll(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len);

/**
 * This writes data to the QSPI flash. If the data spans multiple pages then
 * multiple page program commands will be issued. If ctx->quad_page_program_enable
 * is true, then the command in ctx->quad_page_program_cmd is sent and all
 * four SIO lines are used to send the address and data. Otherwise, the standard
 * page program command is sent and only SIO0 (MOSI) is used to send the address
 * and data.
 *
 * The driver handles sending the write enable command, as well as waiting for
 * the write to complete.
 *
 * This function may return before the write operation is complete, as the actual
 * write operation is queued and executed by a thread created by the driver.
 *
 * \note this function does NOT erase the flash first. Erase operations must be
 * explicitly requested by the application.
 *
 * \param ctx     A pointer to the QSPI flash driver instance to use.
 * \param data    Pointer to the data to write to the flash.
 * \param address The byte address in the flash to begin writing at.
 *                Only bits 23:0 contain the address. The byte in bits 31:24 is
 *                not sent.
 * \param len     The number of bytes to write to the flash.
 */
inline void rtos_qspi_flash_write(
        rtos_qspi_flash_t *ctx,
        const uint8_t *data,
        unsigned address,
        size_t len)
{
    ctx->write(ctx, data, address, len);
}

/**
 * This erases data from the QSPI flash. If the address range to erase
 * spans multiple sectors, then all of these sectors will be erased by issuing
 * multiple erase commands.
 *
 * The driver handles sending the write enable command, as well as waiting for
 * the write to complete.
 *
 * This function may return before the write operation is complete, as the actual
 * erase operation is queued and executed by a thread created by the driver.
 *
 * \note The smallest amount of data that can be erased is a 4k sector.
 * This means that data outside the address range specified by \p address
 * and \p len will be erased if the address range does not both begin and
 * end at 4k sector boundaries.
 *
 * \param ctx     A pointer to the QSPI flash driver instance to use.
 * \param address The byte address to begin erasing. This does not need to begin
 *                at a sector boundary, but if it does not, note that the entire
 *                sector that contains this address will still be erased.
 * \param len     The minimum number of bytes to erase. If \p address + \p len - 1
 *                does not correspond to the last address within a sector, note that
 *                the entire sector that contains this address will still be erased.
 */
inline void rtos_qspi_flash_erase(
        rtos_qspi_flash_t *ctx,
        unsigned address,
        size_t len)
{
    ctx->erase(ctx, address, len);
}

/**
 * This gets the size in bytes of the flash chip.
 *
 * \param A pointer to the QSPI flash driver instance to query.
 *
 * \returns the size in bytes of the flash chip.
 */
inline size_t rtos_qspi_flash_size_get(
        rtos_qspi_flash_t *qspi_flash_ctx)
{
    return qspi_flash_ctx->flash_size;
}

/**
 * This gets the size in bytes of each page in the flash chip.
 *
 * \param A pointer to the QSPI flash driver instance to query.
 *
 * \returns the size in bytes of the flash page.
 */
inline size_t rtos_qspi_flash_page_size_get(
        rtos_qspi_flash_t *qspi_flash_ctx)
{
    return qspi_flash_ctx->ctx.page_size_bytes;
}

/**
 * This gets the number of pages in the flash chip.
 *
 * \param A pointer to the QSPI flash driver instance to query.
 *
 * \returns the number of pages in the flash chip.
 */
inline size_t rtos_qspi_flash_page_count_get(
        rtos_qspi_flash_t *qspi_flash_ctx)
{
    return qspi_flash_ctx->ctx.page_count;
}

/**
 * This gets the sector size of the flash chip
 *
 * \param A pointer to the QSPI flash driver instance to query.
 *
 * \returns the size in bytes of the smallest sector
 */
inline size_t rtos_qspi_flash_sector_size_get(
        rtos_qspi_flash_t *qspi_flash_ctx)
{
    return qspi_flash_erase_type_size(&qspi_flash_ctx->ctx, qspi_flash_erase_1);
}

/**@}*/

/**
 * Starts an RTOS QSPI flash driver instance. This must only be called by the tile that
 * owns the driver instance. It may be called either before or after starting
 * the RTOS, but must be called before any of the core QSPI flash driver functions are
 * called with this instance.
 *
 * rtos_qspi_flash_init() must be called on this QSPI flash driver instance prior to calling this.
 *
 * \param ctx       A pointer to the QSPI flash driver instance to start.
 * \param priority  The priority of the task that gets created by the driver to
 *                  handle the QSPI flash interface.
 */
void rtos_qspi_flash_start(
        rtos_qspi_flash_t *ctx,
        unsigned priority);

/**
 * Initializes an RTOS QSPI flash driver instance.
 * This must only be called by the tile that owns the driver instance. It may be
 * called either before or after starting the RTOS, but must be called before calling
 * rtos_qspi_flash_start() or any of the core QSPI flash driver functions with this instance.
 *
 * \param ctx                            A pointer to the QSPI flash driver instance to initialize.
 * \param clock_block                    The clock block to use for the qspi_io interface.
 * \param cs_port                        The chip select port. MUST be a 1-bit port.
 * \param sclk_port                      The SCLK port. MUST be a 1-bit port.
 * \param sio_port                       The SIO port. MUST be a 4-bit port.
 * \param source_clock                   The source clock to use for the QSPI I/O interface. Must be
 *                                       either qspi_io_source_clock_ref or qspi_io_source_clock_xcore.
 * \param full_speed_clk_divisor         The divisor to use for QSPI reads and writes as well as SPI writes.
 *                                       The frequency of SCLK will be set to:
 *                                       (F_src) / (2 * full_speed_clk_divisor)
 *                                       Where F_src is the frequency of the source clock specified
 *                                       by \p source_clock.
 * \param full_speed_sclk_sample_delay   Number of SCLK cycles to delay the sampling of SIO on input
 *                                       during a full speed transaction.
 *                                       Usually either 0 or 1 depending on the SCLK frequency.
 * \param full_speed_sclk_sample_edge    The SCLK edge to sample the SIO input on during a full speed
 *                                       transaction. May be either qspi_io_sample_edge_rising or
 *                                       qspi_io_sample_edge_falling.
 * \param full_speed_sio_pad_delay       Number of core clock cycles to delay sampling the SIO pads during
 *                                       a full speed transaction. This allows for more fine grained adjustment
 *                                       of sampling time. The value may be between 0 and 5.
 * \param spi_read_clk_divisor           The divisor to use for the clock when performing a SPI read. This
 *                                       may need to be slower than the clock used for writes and QSPI reads.
 *                                       This is because a small handful of instructions must execute to turn
 *                                       the SIO port around from output to input and they must execute within
 *                                       a single SCLK period during a SPI read. QSPI reads have dummy cycles
 *                                       where these instructions may execute which allows for a higher clock
 *                                       frequency.
 *                                       The frequency of SCLK will be set to:
 *                                       (F_src) / (2 * spi_read_clk_divisor)
 *                                       Where F_src is the frequency of the source clock specified
 *                                       by \p source_clock.
 * \param spi_read_sclk_sample_delay     Number of SCLK cycles to delay the sampling of SIO on input
 *                                       during a SPI read transaction.
 *                                       Usually either 0 or 1 depending on the SCLK frequency.
 * \param spi_read_sclk_sample_edge      The SCLK edge to sample the SIO input on during a SPI read
 *                                       transaction. May be either qspi_io_sample_edge_rising or
 *                                       qspi_io_sample_edge_falling.
 * \param spi_read_sio_pad_delay         Number of core clock cycles to delay sampling the SIO pads during
 *                                       a SPI read transaction. This allows for more fine grained adjustment
 *                                       of sampling time. The value may be between 0 and 5.
 * \param quad_page_program_cmd          The command that will be sent when rtos_qspi_flash_write() is called if
 *                                       quad_page_program_enable is true. This should be a value returned by
 *                                       the QSPI_IO_BYTE_TO_MOSI() macro.
 */
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
        qspi_flash_page_program_cmd_t quad_page_program_cmd);

/**@}*/

#endif /* RTOS_QSPI_FLASH_H_ */
