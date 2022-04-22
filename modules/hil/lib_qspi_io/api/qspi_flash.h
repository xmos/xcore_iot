// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#pragma once

/** \file
 *  \brief API for QSPI Flash
 */

#include <stdbool.h>
#include "qspi_io.h"

/**
 * \addtogroup hil_qspi_flash hil_qspi_flash
 *
 * The public API for using HIL QSPI flash I/O.
 * @{
 */

#if !defined(QSPI_FLASH_SANITY_CHECKS)
/**
 * When QSPI_FLASH_SANITY_CHECKS is true then some
 * run-time sanity checks are made at the expense
 * of some extra overhead.
 */
#define QSPI_FLASH_SANITY_CHECKS 0
#endif

typedef enum {
    /**
     * Programs pages using only MOSI. Most, if not all, QSPI flashes support
     * this command. Use this if in doubt.
     */
    qspi_flash_page_program_1_1_1,

    /**
     * Programs pages by sending the command and address over just SIO0,
     * but the data over all four data lines. Many QSPI flashes support either
     * this or qspi_flash_page_program_1_4_4, but not both. Check the datasheet.
     * If the particular flash chip that will be used is unknown, then
     * qspi_flash_page_program_1_1_1 should be used.
     */
    qspi_flash_page_program_1_1_4,

    /**
     * Programs pages by sending the command over just SIO0, but the
     * address and data over all four data lines. Many QSPI flashes support either
     * this or qspi_flash_page_program_1_1_4, but not both. Check the datasheet.
     * If the particular flash chip that will be used is unknown, then
     * qspi_flash_page_program_1_1_1 should be used.
     */
    qspi_flash_page_program_1_4_4,
} qspi_flash_page_program_cmd_t;

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
	 * The type of page program command that will be used when qspi_flash_write() is
	 * called. See qspi_flash_page_program_cmd_t.
	 */
	qspi_flash_page_program_cmd_t quad_page_program_cmd;

	/*
	 * The following members are all set automatically by qspi_flash_init() if
	 * SFDP is supported. These may be set prior to calling qspi_flash_init()
	 * in the event that SFDP is not supported.
     * Additionally, sfdp_skip may be set to skip SFDP and use manually set
     * parameters.
	 */
	bool sfdp_skip;
	bool sfdp_supported;
	size_t page_size_bytes;
	size_t page_count;
	size_t flash_size_kbytes;

	/* should be 3 or 4 */
	int address_bytes;

    struct {
        uint32_t size_log2;
        uint32_t cmd;
    } erase_info[4];

    uint32_t busy_poll_cmd;
    uint8_t busy_poll_bit;
    uint8_t busy_poll_ready_value;

    /* 1 or 2. 0 means quad mode is automatically detected and cannot be explicitly entered */
    uint8_t qe_reg;
    uint8_t qe_bit;
    uint32_t sr2_read_cmd; /* if 0, then read 2 bytes with the standard status register read command */
    uint32_t sr2_write_cmd;/* if 0, then write 2 bytes with the standard status register write command */

} qspi_flash_ctx_t;

/**
 * Most QSPI flashes allow data to be erased in 4k, 32k or 64k chunks,
 * as well as the entire chip. However, these values are not always
 * available on all chips. The erase info table in the qspi flash context
 * structure defines the size of each erasable sector size. This is typically
 * populated automatically by qspi_flash_init() from the SFDP data in the flash.
 */
typedef enum {
	qspi_flash_erase_1,    /**< Erase the first available sector size. This should always be available and will be the smallest available erasable sector size. */
	qspi_flash_erase_2,    /**< Erase the second available sector size. This should be smaller than qspi_flash_erase_3 if both are available. */
	qspi_flash_erase_3,    /**< Erase the third available sector size. This should be smaller than qspi_flash_erase_4 if both are available. */
	qspi_flash_erase_4,    /**< Erase the fourth available sector size. This is typically not available, but will be the largest available erasable sector size if it is. */
	qspi_flash_erase_chip, /**< Erase the entire chip */
} qspi_flash_erase_length_t;

/**
 * The bit mask for the status register's write
 * in progress bit.
 */
#define QSPI_FLASH_STATUS_REG_WIP_BM 0x01

/**
 * The bit mask for the status register's write
 * enable latch bit.
 */
#define QSPI_FLASH_STATUS_REG_WEL_BM 0x02

/*
 * Returns the erase size in bytes associated with the given erase type.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 * \param erase_type The erase type to return the size of.
 *
 * \returns The erase size in bytes of \p erase type. If \p erase_type
 * is qspi_flash_erase_chip then SIZE_MAX is returned. If \p erase_type
 * is invalid or not available on the flash chip, then 0 is returned.
 */
inline size_t qspi_flash_erase_type_size(qspi_flash_ctx_t *ctx, qspi_flash_erase_length_t erase_type)
{
    if (erase_type >= qspi_flash_erase_1 && erase_type <= qspi_flash_erase_4) {
        uint32_t size_log2 = ctx->erase_info[erase_type].size_log2;
        return size_log2 > 0 ? (1 << size_log2) : 0;
    } else if (erase_type == qspi_flash_erase_chip) {
        return SIZE_MAX;
    } else {
        return 0;
    }
}

/*
 * Returns log2 of the erase size in bytes associated with the given erase type.
 *
 * \param ctx The QSPI flash context associated with the QSPI flash.
 * \param erase_type The erase type to return the size of.
 *
 * \returns The log2 of the erase size in bytes of \p erase type. If \p erase_type
 * is qspi_flash_erase_chip then UINT32_MAX is returned. If \p erase_type
 * is invalid or not available on the flash chip, then 0 is returned.
 */
inline uint32_t qspi_flash_erase_type_size_log2(qspi_flash_ctx_t *ctx, qspi_flash_erase_length_t erase_type)
{
    if (erase_type >= qspi_flash_erase_1 && erase_type <= qspi_flash_erase_4) {
        return ctx->erase_info[erase_type].size_log2;
    } else if (erase_type == qspi_flash_erase_chip) {
        return UINT32_MAX;
    } else {
        return 0;
    }
}

/**
 * Sets or clears the quad enable bit in the flash.
 *
 * \note The quad enable bit is fixed to '1' in some QSPI flash
 * chips, and cannot be cleared.
 *
 * \param ctx  The QSPI flash context associated with the QSPI flash.
 * \param set  When true, the quad enable bit is set. When false,
 *             the quad enable bit is cleared if possible.
 *
 * \retval true if the QE bit was already at the requested value,
 *         or if the write was successful.
 * \retval false if the write did not complete successfully. This
 *         can happen when trying to clear the QE bit on parts where
 *         it is fixed to '1'.
 */
bool qspi_flash_quad_enable_write(qspi_flash_ctx_t *ctx, bool set);

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
 *
 * \retval true if there is a flash write in progress.
 * \retval false if the flash is not writing and is ready to accept another
 *         read or write command.
 */
bool qspi_flash_write_in_progress(qspi_flash_ctx_t *ctx);

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
 * This reads data from the flash in fast mode. This is a normal SPI read,
 * using only SIO0 (MOSI) and SIO1 (MOSI), but includes eight dummy cycles
 * between the address and data.
 *
 * \param ctx     The QSPI flash context associated with the QSPI flash.
 * \param data    Pointer to the buffer to save the read data to.
 * \param address The byte address in the flash to begin reading at.
 *                Only bits 23:0 contain the address. The byte in bits 31:24
 *                is not sent.
 * \param len     The number of bytes to read and save to \p data.
 */
void qspi_flash_fast_read(qspi_flash_ctx_t *ctx,
                          uint8_t *data,
                          uint32_t address,
                          size_t len);

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
 *                Only bits 23:0 contain the address. The byte in bits 31:24
 *                is not sent.
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
 * This reads the SFDP data from the flash in 1-1-1 mode.
 *
 * \param ctx     The QSPI flash context associated with the QSPI flash.
 * \param data    Pointer to the buffer to save the read data to.
 * \param address The byte address in the SFDP area to begin reading at.
 *                Only bits 23:0 contain the address. The byte in bits 31:24
 *                is not sent.
 * \param len     The number of bytes to read and save to \p data.
 */
void qspi_flash_sfdp_read(qspi_flash_ctx_t *ctx,
                          uint8_t *data,
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

/**@}*/ // END: addtogroup hil_qspi_flash
