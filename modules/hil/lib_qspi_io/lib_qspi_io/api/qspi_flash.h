// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1
#pragma once

/** \file
 *  \brief API for QSPI Flash
 */

#include "qspi_io.h"

#if !defined(QSPI_FLASH_SANITY_CHECKS)
/**
 * When QSPI_FLASH_SANITY_CHECKS is true then some
 * run-time sanity checks are made at the expense
 * of some extra overhead.
 */
#define QSPI_FLASH_SANITY_CHECKS 0
#endif

/**
 * The context structure that must be passed to each of the qspi_flash functions. 
 */
typedef struct {
	/**
	 * The context for the QSPI I/O interface that is used by the
	 * QSPI flash. At a minimum, the ports and clock block must be
	 * set prior to calling qspi_flash_init().
	 */
	qspi_io_ctx_t qspi_io_ctx;

	/**
	 * The source clock to use for the QSPI I/O interface. Must be
	 * either qspi_io_source_clock_ref or qspi_io_source_clock_xcore.
	 * This must be set prior to calling qspi_flash_init().
	 */
	qspi_io_source_clock_t source_clock;

	/**
	 * If set to false, then qspi_flash_init() will setup safe default
	 * values for the QSPI I/O clock configuration. If set to true, then
	 * the application must supply the clock setup values.
	 */
	int custom_clock_setup;

	/**
	 * If set to 1, then qspi_flash_write() will use all four SIO lines
	 * to send out the address and data. If set greater than 1, then qspi_flash_write()
	 * will send the address out on only SIO0 (MOSI) and the data out on all four SIO
	 * lines. Otherwise, if false then both the address and data will be sent
	 * out on only SIO0 (MOSI).
	 */
	int quad_page_program_enable;

	/**
	 * The command that will be sent when qspi_flash_write() is called if
	 * quad_page_program_enable is true. This should be a value returned by
	 * the QSPI_IO_BYTE_TO_MOSI() macro.
	 */
	uint32_t quad_page_program_cmd;
} qspi_flash_ctx_t;

/**
 * Most QSPI flashes allow data to be erased in 4k, 32k or 64k chunks,
 * as well as the entire chip.
 */
typedef enum {
	qspi_flash_erase_4k,   /**< Erase a 4k byte sector */
	qspi_flash_erase_32k,  /**< Erase a 32k byte block */
	qspi_flash_erase_64k,  /**< Erase a 64k byte block */
	qspi_flash_erase_chip, /**< Erase the entire chip */
} qspi_flash_erase_length_t;

/**
 * The bit mask for the status register's write
 * in progress bit.
 */
#define QSPI_FLASH_STATUS_REG_WIP_BM 0x00000001

/**
 * The bit mask for the status register's write
 * enable latch bit.
 */
#define QSPI_FLASH_STATUS_REG_WEL_BM 0x00000002

/**
 * Sets the write enable latch in the QSPI flash. This must be called
 * prior to erasing, programming, or writing to a register.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 */
void qspi_flash_write_enable(qspi_flash_ctx_t *ctx);

/**
 * This clears the write enable latch in the QSPI flash. This is cleared
 * automatically at the end of write operations, so normally does not need
 * to be called.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 */
void qspi_flash_write_disable(qspi_flash_ctx_t *ctx);

/**
 * This checks to see if the QSPI flash has a write operation in progress.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 */
int qspi_flash_write_in_progress(qspi_flash_ctx_t *ctx);

/**
 * This waits while the QSPI flash has a write operation in progress.
 * It returns when the write operation is complete, or immediately if
 * there is not one in progress to begin with.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 */
void qspi_flash_wait_while_write_in_progress(qspi_flash_ctx_t *ctx);

/**
 * This performs an erase operation. qspi_flash_write_enable() must be called
 * prior to this.
 *
 * \param ctx          The QSPI flash context associated with the QSPI flash.
 * \param address      Any byte address within the data block to erase.
 * \param erase_length The data block size to erase. See qspi_flash_erase_length_t.
 *                     If qspi_flash_erase_chip, then \p address is ignored.
 */
void qspi_flash_erase(qspi_flash_ctx_t *ctx,
                      uint32_t address,
                      qspi_flash_erase_length_t erase_length);

/**
 * This writes to a register in the QSPI flash. This allows an application
 * to write to non-standard registers specific to its flash chip.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 * \param cmd The command required for writing to the desired register.
              Must be the value returned by QSPI_IO_BYTE_TO_MOSI().
 * \param val Pointer to the data to write to the register.
 * \param len The number of bytes from \p val to write to the register.
 */
void qspi_flash_write_register(qspi_flash_ctx_t *ctx,
                               uint32_t cmd,
                               const uint8_t *val,
                               size_t len);

/**
 * This writes to the status register in the QSPI flash. qspi_flash_write_enable()
 * must be called prior to this.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 * \param val Pointer to the data to write to the register.
 * \param len The number of bytes from \p val to write to the register.
 */
void qspi_flash_write_status_register(qspi_flash_ctx_t *ctx,
                                      const uint8_t *val,
                                      size_t len);

/**
 * This reads from a register in the QSPI flash. This allows an application
 * to read from non-standard registers specific to its flash chip.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 * \param cmd The command required for reading from the desired register.
              Must be the value returned by QSPI_IO_BYTE_TO_MOSI().
 * \param val Pointer to the buffer to store the data read from the register.
 * \param len The number of bytes to read from the register and save to \p val.
 */
void qspi_flash_read_register(qspi_flash_ctx_t *ctx,
                              uint32_t cmd,
                              uint8_t *val,
                              size_t len);

/**
 * This reads from the status register in the QSPI flash.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 * \param val Pointer to the buffer to store the data read from the register.
 * \param len The number of bytes to read from the register and save to \p val.
 */
void qspi_flash_read_status_register(qspi_flash_ctx_t *ctx,
                                     uint8_t *val,
                                     size_t len);

/**
 * This reads the JEDEC ID of the QSPI flash.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 * \param val Pointer to the buffer to write the ID to.
 * \param len The number of ID bytes to read and save to \p val.
 */
void qspi_flash_read_id(qspi_flash_ctx_t *ctx,
                        uint8_t *val,
                        size_t len);

/**
 * This polls a register in the QSPI flash. This allows an application to
 * poll non-standard registers specific to its flash chip. The register must
 * be one byte and repeatedly sent out over MISO following the read register
 * command.
 *
 * \param ctx   The QSPI flash context associated with the QSPI flash.
 * \param cmd   The command required for reading from the desired register.
                Must be the value returned by QSPI_IO_BYTE_TO_MOSI().
 * \param mask  The bitmask to apply to the received register value before
 *              comparing it to \p val;
 * \param val   The value that the register value, masked with \p mask, must
 *              match before returning.
 */
void qspi_flash_poll_register(qspi_flash_ctx_t *ctx,
                              uint32_t cmd,
                              const uint8_t mask,
                              const uint8_t val);

/**
 * This polls the status register in the QSPI flash.
 *
 * \param ctx   The QSPI flash context associated with the QSPI flash.
 * \param mask  The bitmask to apply to the received register value before
 *              comparing it to \p val;
 * \param val   The value that the register value, masked with \p mask, must
 *              match before returning.
 */
void qspi_flash_poll_status_register(qspi_flash_ctx_t *ctx,
                                     const uint8_t mask,
                                     const uint8_t val);

/**
 * This reads data from the flash in quad I/O mode. All four lines are
 * used to send the address and to read the data.
 *
 * \param ctx     The QSPI flash context associated with the QSPI flash.
 * \param data    Pointer to the buffer to save the read data to.
 * \param address The byte address in the flash to begin reading at.
 *                Only bits 23:0 contain the address. Bits 31:24 are actually
 *                transmitted to the flash during the first two dummy cycles
 *                following the three address bytes. Some flashes read the SIO
 *                lines during these first two dummy cycles to enable certain
 *                features, so this might be useful for some applications.
 * \param len     The number of bytes to read and save to \p data.
 */
void qspi_flash_read(qspi_flash_ctx_t *ctx,
                     uint8_t *data,
                     uint32_t address,
                     size_t len);

/**
 * This is the same as qspi_flash_read() except that the nibbles in each
 * byte of the data returned are swapped.
 *
 * \param ctx     The QSPI flash context associated with the QSPI flash.
 * \param data    Pointer to the buffer to save the read data to.
 * \param address The byte address in the flash to begin reading at.
 *                Only bits 23:0 contain the address. Bits 31:24 are actually
 *                transmitted to the flash during the first two dummy cycles
 *                following the three address bytes. Some flashes read the SIO
 *                lines during these first two dummy cycles to enable certain
 *                features, so this might be useful for some applications.
 * \param len     The number of bytes to read and save to \p data.
 */
void qspi_flash_read_nibble_swapped(qspi_flash_ctx_t *ctx,
                                    uint8_t *data,
                                    uint32_t address,
                                    size_t len);

/**
 * This reads data from the flash in quad I/O "eXecute In Place" mode.
 * All four lines are used to send the address and to read the data.
 * No command is sent. The flash must already have been put into "XIP" mode.
 
 * The method used to put the flash into XIP mode, as well as to take it out,
 * is flash dependent. See your flash's datasheet.
 *
 * \param ctx     The QSPI flash context associated with the QSPI flash.
 * \param data    Pointer to the buffer to save the read data to.
 * \param address The byte address in the flash to begin reading at.
 *                Only bits 23:0 contain the address. Bits 31:24 are actually
 *                transmitted to the flash during the first two dummy cycles
 *                following the three address bytes. Some flashes read the SIO
 *                lines during these first two dummy cycles to enable certain
 *                features, so this might be useful for some applications.
 * \param len     The number of bytes to read and save to \p data.
 */
void qspi_flash_xip_read(qspi_flash_ctx_t *ctx,
                         uint8_t *data,
                         uint32_t address,
                         size_t len);

/**
 * This is the same as qspi_flash_xip_read() except that the nibbles in each
 * byte of the data returned are swapped.
 *
 * \param ctx     The QSPI flash context associated with the QSPI flash.
 * \param data    Pointer to the buffer to save the read data to.
 * \param address The byte address in the flash to begin reading at.
 *                Only bits 23:0 contain the address. Bits 31:24 are actually
 *                transmitted to the flash during the first two dummy cycles
 *                following the three address bytes. Some flashes read the SIO
 *                lines during these first two dummy cycles to enable certain
 *                features, so this might be useful for some applications.
 * \param len     The number of bytes to read and save to \p data.
 */
void qspi_flash_xip_read_nibble_swapped(qspi_flash_ctx_t *ctx,
                                        uint8_t *data,
                                        uint32_t address,
                                        size_t len);

/**
 * This writes data to a page in the QSPI flash. If ctx->quad_page_program_enable
 * is true, then the command in ctx->quad_page_program_cmd is sent and all
 * four SIO lines are used to send the address and data. Otherwise, the standard
 * page program command is sent and only SIO0 (MOSI) is used to send the address
 * and data.
 *
 * qspi_flash_write_enable() must be called prior to this.
 *
 * \param ctx     The QSPI flash context associated with the QSPI flash.
 * \param data    Pointer to the data to write to the flash.
 * \param address The byte address in the flash to begin writing at.
 *                Only bits 23:0 contain the address. The byte in bits 31:0 is
 *                not sent.
 * \param len     The number of bytes to write to the flash.
 */
void qspi_flash_write(qspi_flash_ctx_t *ctx,
                      const uint8_t *data,
                      uint32_t address,
                      size_t len);
                      
/**
 * This is the same as qspi_flash_write() except that the nibbles in each
 * byte of the data written are swapped.
 *
 * qspi_flash_write_enable() must be called prior to this.
 *
 * \param ctx     The QSPI flash context associated with the QSPI flash.
 * \param data    Pointer to the data to write to the flash.
 * \param address The byte address in the flash to begin writing at.
 *                Only bits 23:0 contain the address. The byte in bits 31:0 is
 *                not sent.
 * \param len     The number of bytes to write to the flash.
 */
void qspi_flash_write_nibble_swapped(qspi_flash_ctx_t *ctx,
                                     const uint8_t *data,
                                     uint32_t address,
                                     size_t len);

/**
 * Deinitializes the QSPI I/O interface associated with the QSPI flash.
 * 
 * \param ctx The QSPI flash context associated with the QSPI flash.
 *            This should have been previously initialized with
 *            qspi_flash_init().
 */
void qspi_flash_deinit(qspi_flash_ctx_t *ctx);

/**
 * Initializes the QSPI I/O interface associated with the QSPI flash.
 * The ports and clock block in the ctx->qspi_io_ctx must be set prior
 * to calling this.
 *
 * If ctx->custom_clock_setup is false, then the QSPI I/O clock configuration
 * is set to safe default values that should work for all QSPI flashes.
 * Otherwise, the clock configuration values must be supplied by the application.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 */
void qspi_flash_init(qspi_flash_ctx_t *ctx);

