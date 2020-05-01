// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <xcore/triggerable.h>
#include <xcore/chanend.h>
#include <xmos_flash.h>
#include <timer.h>
#include <string.h>

#include "rtos_support.h"
#include "soc.h"

#include "qspi_flash_dev.h"

#define WORDS_TO_BYTES(w) ((w) * sizeof(uint32_t))
#define BYTES_TO_WORDS(b) (((b) + sizeof(uint32_t) - 1) / sizeof(uint32_t))

#define WORD_TO_BYTE_ADDRESS(w) WORDS_TO_BYTES(w)
#define BYTE_TO_WORD_ADDRESS(b) ((b) / sizeof(uint32_t))


#define SECTORS_TO_BYTES(s, ss) ((s) * (ss))
#define BYTES_TO_SECTORS(b, ss) (((b) + (ss) - 1) / (ss))

#define SECTOR_TO_BYTE_ADDRESS(s, ss) SECTORS_TO_BYTES(s, ss)
#define BYTE_TO_SECTOR_ADDRESS(b, ss) ((b) / (ss))


#define DEFAULT_PAGE_COUNT 16384

#define ERASE_SIZE_4K  4096
#define ERASE_SIZE_16K 16384
#define ERASE_SIZE_32K 32768
static const int erase_sizes[] = {ERASE_SIZE_4K, ERASE_SIZE_16K, ERASE_SIZE_32K};

static const int page_size = 256;

/*
 * TODO: need to be able to set page count via the driver.
 * which should update flash_size_bytes and flash_size_words
 */
static int page_count = DEFAULT_PAGE_COUNT;
static int flash_size_bytes = 256 * DEFAULT_PAGE_COUNT;
static int flash_size_words = BYTES_TO_WORDS(256 * DEFAULT_PAGE_COUNT);


static int is_busy(const flash_handle_t *flash_handle)
{
	char status;
	status = flash_read_status_register(flash_handle, flash_status_register_0);
	return (status & 0x1) == 1;
}

static void wait_while_busy(const flash_handle_t *flash_handle)
{
	while(is_busy(flash_handle)) {
		delay_microseconds(1);
	}
}

static void enable_quad_mode(
		const flash_handle_t    *flash_handle,
		const flash_qe_config_t *flash_qe_config)
{
	char quad_enable[2] = {0x00, 0x00};
	flash_num_status_bytes_t num_status_bytes;

	if(flash_qe_config->flash_qe_location == flash_qe_location_status_reg_0) {
		quad_enable[0] = (1 << flash_qe_config->flash_qe_shift);
		num_status_bytes = flash_num_status_bytes_1;
	} else {
		quad_enable[1] = (1 << flash_qe_config->flash_qe_shift);
		num_status_bytes = flash_num_status_bytes_2;
	}

	flash_write_enable(flash_handle);
	flash_write_status_register(flash_handle, quad_enable, num_status_bytes);
	wait_while_busy(flash_handle);
}

static int is_quad_mode_enabled(
		const flash_handle_t    *flash_handle,
		const flash_qe_config_t *flash_qe_config)
{
    uint8_t status;

    status = flash_read_status_register(flash_handle, (flash_status_register_t) flash_qe_config->flash_qe_location);

    return (status & (1 << flash_qe_config->flash_qe_shift)) != 0;
}

typedef struct  {
	qspi_flash_dev_cmd_t cmd;
	union {
		uint32_t word_buf[BYTES_TO_WORDS(QSPI_FLASH_DEV_WRITE_BUFSIZE)];
		uint8_t byte_buf[QSPI_FLASH_DEV_WRITE_BUFSIZE];
	};
} recv_buf_t;


static uint8_t *op_read(
		const flash_handle_t *flash_handle,
		recv_buf_t *buf)
{
	int word_address;
	int word_count;
	int front_pad_len;

	word_address = BYTE_TO_WORD_ADDRESS(buf->cmd.byte_address);
	front_pad_len = buf->cmd.byte_address - WORD_TO_BYTE_ADDRESS(word_address);
	word_count = BYTES_TO_WORDS(front_pad_len + buf->cmd.byte_count);

	if (word_address + word_count > flash_size_words) {
		int original_word_count = word_count;

		/* Don't read past the end of the flash */
		word_count = flash_size_words - word_address;

		/* Return all 0xFF bytes for addresses beyond the end of the flash */
		memset(&buf->word_buf[word_count], 0xFF, original_word_count - word_count);
	}

	rtos_printf("Read %u words from flash at address 0x%x\n", word_count, word_address);
	flash_read_quad(flash_handle, word_address, buf->word_buf, word_count);

	return &buf->byte_buf[front_pad_len];
}

static void op_write(
		const flash_handle_t *flash_handle,
		recv_buf_t *buf)
{
#if QSPI_FLASH_QUAD_PAGE_PROGRAM
	int word_address;
	int word_count;
	int front_pad_len;
	int end_pad_start;

	word_address = BYTE_TO_WORD_ADDRESS(buf->cmd.byte_address);
	front_pad_len = buf->cmd.byte_address - WORD_TO_BYTE_ADDRESS(word_address);
	end_pad_start = front_pad_len + buf->cmd.byte_count;
	word_count = BYTES_TO_WORDS(end_pad_start);

	/*
	 * If lib_flash ever supports 4x I/O page programming, then it will
	 * likely require word address and word length. Enabling this padding
	 * will support that.
	 */

	/*
	 * Ensure that the front and back of the buffer are padded with 0xFFs.
	 * It is the software driver's responsibility to ensure at least
	 * that the beginning of the data is properly aligned.
	 * The device here pads the end, and also sets the first pad bytes to 0xFF.
	 * For example, if writing 5 bytes to byte addres 2, then the received buffer should
	 * contain: 0xFF, 0xFF, B0, B1, B2, B3, B4, 0xFF
	 * the padding in the beginning can be accomplished with a pad TX buffer between the
	 * flash command struct and the data.
	 */
	rtos_printf("Pad front with %d bytes\n", front_pad_len);
	memset(buf->byte_buf, 0xFF, front_pad_len);
	rtos_printf("Pad end starting at address 0x%08x with %d bytes\n", end_pad_start, WORDS_TO_BYTES(word_count) - end_pad_start);
	memset(&buf->byte_buf[end_pad_start], 0xFF, WORDS_TO_BYTES(word_count) - end_pad_start);

	/*
	 * TODO: Will needs to handle page boundaries like below.
	 */
	flash_write_quad(flash_handle, word_address, buf->word_buf, word_count);
#else
	int bytes_left_to_write = buf->cmd.byte_count;
	int address_to_write = buf->cmd.byte_address;
	uint8_t *write_buf = buf->byte_buf;

	while (bytes_left_to_write > 0) {
		/* compute the maximum number of bytes that can be written to the current page. */
		int max_bytes_to_write = page_size - address_to_write % page_size;
		int bytes_to_write = bytes_left_to_write <= max_bytes_to_write ? bytes_left_to_write : max_bytes_to_write;

		if (address_to_write >= flash_size_bytes) {
			break; /* do not write past the end of the flash */
		}

		flash_write_enable(flash_handle);
		flash_write_page(flash_handle, address_to_write, write_buf, bytes_to_write);
		wait_while_busy(flash_handle);
		bytes_left_to_write -= bytes_to_write;
		write_buf += bytes_to_write;
		address_to_write += bytes_to_write;
	}
#endif
}

static void op_erase(
		const flash_handle_t *flash_handle,
		recv_buf_t *buf)
{
	int bytes_left_to_erase = buf->cmd.byte_count;
	int address_to_erase = buf->cmd.byte_address;

	rtos_printf("Asked to erase %u bytes at address 0x%08x\n", bytes_left_to_erase, address_to_erase);

	if (address_to_erase == 0 && bytes_left_to_erase >= flash_size_bytes) {
		/* Use chip erase when being asked to erase the entire address range */
		rtos_printf("Erasing entire chip\n");
		flash_write_enable(flash_handle);
		flash_erase_chip(flash_handle);
		wait_while_busy(flash_handle);
	} else {

		if (SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(address_to_erase, erase_sizes[0]), erase_sizes[0]) != address_to_erase) {
			/*
			 * If the provided starting erase address does not begin on the smallest
			 * sector boundary, then update the starting address and number of bytes
			 * to erase so that it does.
			 */
			int sector_address;
			sector_address = BYTE_TO_SECTOR_ADDRESS(address_to_erase, erase_sizes[0]);
			bytes_left_to_erase += address_to_erase - SECTOR_TO_BYTE_ADDRESS(sector_address, erase_sizes[0]);
			address_to_erase = SECTOR_TO_BYTE_ADDRESS(sector_address, erase_sizes[0]);
			rtos_printf("adjusted starting erase address to %d\n", address_to_erase);
		}

		while (bytes_left_to_erase > 0) { /* bytes_left_to_erase may go negative */
			int erase_length = erase_sizes[0];
			int sector_address;

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

			sector_address = BYTE_TO_SECTOR_ADDRESS(address_to_erase, erase_length);
			xassert(address_to_erase == SECTOR_TO_BYTE_ADDRESS(sector_address, erase_length));

			rtos_printf("Erasing %d bytes (%d) at byte address %u (sector address %u)\n", erase_length, bytes_left_to_erase, address_to_erase, sector_address);

			flash_write_enable(flash_handle);
			switch (erase_length) {
			case ERASE_SIZE_4K:
				flash_erase_sector(flash_handle, sector_address);
				break;

			case ERASE_SIZE_16K:
				flash_erase_block_32KB(flash_handle, sector_address);
				break;

			case ERASE_SIZE_32K:
				flash_erase_block_64KB(flash_handle, sector_address);
				break;

			default:
				xassert(0);
			}
			wait_while_busy(flash_handle);

			address_to_erase += erase_length;
			bytes_left_to_erase -= erase_length;
		}
	}
}

static uint8_t *handle_op_request(
		const flash_handle_t *flash_handle,
		recv_buf_t *buf,
		int recv_len)
{
	uint8_t *return_buf = NULL;

	/* Ensure that at least a full command was sent */
	xassert(recv_len >= sizeof(buf->cmd));

	switch (buf->cmd.operation) {
	case qspi_flash_dev_op_read:
		/*
		 * Ensure that the requested number of bytes to read will
		 * fit inside the word buffer.
		 */
		xassert(buf->cmd.byte_count <= sizeof(buf->word_buf));

		return_buf = op_read(flash_handle, buf);
		break;

	case qspi_flash_dev_op_write:
		/*
		 * Ensure that we received at least the requested
		 * number of bytes to write.
		 */
		xassert(recv_len - sizeof(qspi_flash_dev_cmd_t) >= buf->cmd.byte_count);

		op_write(flash_handle, buf);
		break;

	case qspi_flash_dev_op_erase:

		op_erase(flash_handle, buf);
		break;

	default:
		break;
	}

	return return_buf;
}

void qspi_flash_dev(
        soc_peripheral_t peripheral,
		chanend data_to_dma_c,
        chanend data_from_dma_c,
		chanend ctrl_c,
		const flash_ports_t        *flash_ports,
		const flash_clock_config_t *flash_clock_config,
		const flash_qe_config_t    *flash_qe_config)
{
	flash_handle_t flash_handle;
	uint8_t *return_buf;
    uint32_t cmd;
    recv_buf_t buf;
    int len;

    flash_connect(&flash_handle, flash_ports, *flash_clock_config, *flash_qe_config);

    /*
     * Ensure that the quad spi flash is in quad mode
     */
    if (!is_quad_mode_enabled(&flash_handle, flash_qe_config)) {
        debug_printf("quad mode not enabled!\n");
        enable_quad_mode(&flash_handle, flash_qe_config);
        xassert(is_quad_mode_enabled(&flash_handle, flash_qe_config));
        debug_printf("quad mode enabled!\n");
    } else {
        debug_printf("quad mode already enabled!\n");
    }

    triggerable_disable_all();

    if (data_from_dma_c != 0) {
    	TRIGGERABLE_SETUP_EVENT_VECTOR(data_from_dma_c, event_data_from_dma);
    	triggerable_enable_trigger(data_from_dma_c);
    }
    if (ctrl_c != 0) {
    	TRIGGERABLE_SETUP_EVENT_VECTOR(ctrl_c, event_ctrl);
    	triggerable_enable_trigger(ctrl_c);
    }

    for (;;) {

		if (peripheral != NULL) {
			if ((len = soc_peripheral_rx_dma_direct_xfer(peripheral, &buf, sizeof(buf))) >= 0) {

				return_buf = handle_op_request(&flash_handle, &buf, len);
				if (return_buf != NULL) {
					rtos_printf("Reply directly with read data now\n");
					soc_peripheral_tx_dma_direct_xfer(peripheral, return_buf, buf.cmd.byte_count);
				}
			}
		}

		TRIGGERABLE_WAIT_EVENT(event_ctrl, event_data_from_dma);

		event_ctrl: {
			soc_peripheral_function_code_rx(ctrl_c, &cmd);
			switch( cmd ) {
			case SOC_PERIPHERAL_DMA_TX:
				/*
				 * The application has added a new DMA TX buffer. This
				 * ensures that this select statement wakes up and gets
				 * the TX data in the code above.
				 */
				break;

			default:
				rtos_printf( "Invalid CMD\n" );
				break;
			}
			continue;
		}

		event_data_from_dma: {
			soc_peripheral_rx_dma_ready(data_from_dma_c);
			len = soc_peripheral_rx_dma_xfer(data_from_dma_c, &buf, sizeof(buf));

			return_buf = handle_op_request(&flash_handle, &buf, len);
			if (return_buf != NULL) {
				rtos_printf("Reply remotely with read data now\n");
				soc_peripheral_tx_dma_xfer(data_to_dma_c, return_buf, buf.cmd.byte_count);
			}

			continue;
		}
    }
}
