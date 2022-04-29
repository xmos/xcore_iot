//// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _ssd1306_h_
#define _ssd1306_h_

#include <sys/types.h>
#include <stdint.h>

/**
 * Defines the known display types.
 */
typedef enum {
        ssd1306_MDOB128032GV,  /**< A Midas MDOB128032GV (-BI, -WI or -YI variant)
                               * 128x32 pixel display.
                               */
        ssd1306_END
} ssd1306_display_type;

// Define the types of transfer to the ssd1306. D/C# = 0 for commands
#define SSD1306_SEND_COMMAND (0x00)
#define SSD1306_SEND_DATA    (0x40)

// Define commands used by the ssd1306
#define SSD1306_SET_PAGE     (0xB0)

#define SSD1306_END_OF_LIST  (255)

// Define maximum sizes for the SSD1306
#define SSD1306_COLUMNS      (128)
#define SSD1306_PAGES        (8)

/* Defines the structure of an initialisation command
 * The first element in the array is the number of bytes to be sent (1-3)
 * The following bytes contain the values to be sent. Unused elements are
 * ignored.
 *
 * For example, to send the Set Page Address command (See SSD1306 data sheet for
 * details) we need to send 3 bytes: 0x22 followed by two values in the range 0-7.
 * The ssd1306_init_command might be {3, 0x22, 1, 2}, where 1 and 2 are example
 * start and end page numbers.
 */
#define SSD1306_MAX_COMMAND_LENGTH 3
typedef uint8_t ssd1306_init_command[SSD1306_MAX_COMMAND_LENGTH + 1];

/* Defines a translation function that generates a byte to be written to the
 * display given a display buffer. This is specific to the pariticular display the
 * SSD1306 device is connected to.
 */
typedef uint8_t (*ssd1306_translator)(const uint8_t page, const uint8_t column, const uint8_t* const buffer);

typedef struct {
    uint8_t rows;
    uint8_t columns;

    __attribute__(( fptrgroup("ssd1306_display_translator", 1) ))
    ssd1306_translator translator;
    ssd1306_init_command* initialisation; // An ordered list of initialisation commands
} ssd1306_display;


// returns number of bytes written
typedef size_t (*transport_write)(void* app_ctx, void* bus, int address, uint8_t *buf, size_t len);

// Default transport write function
//extern transport_write ssd1306_I2C_write;
size_t ssd1306_I2C_write(void* app_ctx, void* bus, int address, uint8_t *buf, size_t len);

typedef struct {
    void* bus;
    int address; // The address for the display device. The meaing of this is transport dependent

    __attribute__(( fptrgroup("ssd1306_transport_write", 1) ))
    transport_write write;
} ssd1306_transport;

typedef struct {
    const ssd1306_display* display;
    const ssd1306_transport* transport; // returns number of bytes written
} ssd1306_context;

/**
 * Implements an ssd1306 on an xcore_sdk hil I2C interface.
 *
 * \param app_ctx             A pointer to an application context that will be passed to the
 *                            transport write function. Default implementations of the transport
 *                            write do not use this and it can be set to NULL.
 * \param ssd1306             A pointer to the ssd1306 context object to initialise.
 * \param transport           A pointer to the transport context to use. This defines
 *                            the way that the display is accessed.
 * \param type                The display type to use.
 */
void ssd1306_init(
        void* app_ctx,
        ssd1306_context* const ssd1306,
        const ssd1306_transport* const transport,
        const ssd1306_display_type type);

/**
 * Writes a whole buffer to the display. The size of the buffer is determined by the display size
 * as defined in the display_type given to the ssd1306_init function.
 * \param app_ctx             A pointer to an application context that will be passed to the
 *                            transport write function. Default implementations of the transport
 *                            write do not use this and it can be set to NULL.
 * \param ssd1306_ctx         A pointer to a ssd1306_context which has already been successfully
 *                            initialised using ssd1306_init().
 * \param buf                 A pointer to buffer holding the data to be displayed.
 *                            The buffer should be (display columns * display rows) / 8
 *                            entries. The bits to display should be ordered from top
 *                            left scanning along the rows with each uint8_t holding the
 *                            value for 8 consecutive pixels in the row starting at bit
 *                            zero. E.g. for a 128 column, 32 row display:
 *                                                          Column
 *                                         0                1                    127
 *                                 +-----------------+-----------------+     +-----------------+
 *                               0 | Entry   0 bit 0 | Entry   0 bit 1 |     | Entry  15 bit 7 | ...
 *                                 +-----------------+-----------------+     +-----------------+
 *                               1 | Entry  16 bit 0 | Entry  16 bit 1 | ... | Entry  31 bit 7 |
 *                                 +-----------------+-----------------+     +-----------------+
 *                           Row                                         .
 *                                                                       .
 *                                                                       .
 *                                 +-----------------+-----------------+     +-----------------+
 *                              31 | Entry 496 bit 0 | Entry 496 bit 1 |     | Entry 511 bit 7 |
 *                                 +-----------------+-----------------+     +-----------------+
 */
void ssd1306_write(
        void* app_ctx,
        const ssd1306_context* const ssd1306_ctx,
        const uint8_t* const buf);

#endif // _ssd1306_h_
