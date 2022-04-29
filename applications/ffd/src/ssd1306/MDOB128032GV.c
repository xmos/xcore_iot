//// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "ssd1306.h"
#include "MDOB128032GV.h"

#ifndef RPI
/* System headers */
#include <xs1.h>


/* SDK headers */
//#include "xcore_utils.h"
#endif

ssd1306_init_command MDOB128032GV_init_buf[] = {
    {1,0xAE,0,0},    // Display Off
    {2,0xD5,0x80,0}, // Set Display Clock: 370k/1 typical
    {2,0xA8,0x3F,0}, // Set Multiplex Ratio: 64MUX
    {2,0xD3,0x00,0}, // Set Display Offset: 0
    {1,0x40,0,0},    // Set Display Start Line: 0
    {2,0x8D,0x14,0}, // Set Charge Pump: enabled
    {1,0xA1,0,0},    // Set Segment Re-Map: column address 127 is mapped to SEG0
    {1,0xC8,0,0},    // Set COM Output Scan Direction: remapped mode. Scan from COM[N-1] to COM0 where N is the Multiplex ratio.
    {2,0xDA,0x12,0}, // Set COM Hardware Configuration: Alternative COM pin configuration, Disable COM Left/Right remap
    {2,0x81,0x4F,0}, // Set Contrast Control: Maximum
    {2,0xD9,0x22,0}, // Set Pre-Charge period: default
    {2,0xDB,0x30,0}, // Set Deselect Vcomh level: ~ 0.83 x Vcc
    {1,0x00,0,0},
    {1,0x12,0,0},
    {1,0xA4,0,0},    // Entire Display ON
    {1,0xA6,0,0},    // Set Normal Display
    {1,0xAF,0,0},    // Display ON
    {SSD1306_END_OF_LIST,0,0,0}
};

// XCORE only attribute
#ifndef RPI
__attribute__(( fptrgroup("ssd1306_display_translator") ))
#endif
uint8_t MDOB128032GV_translator (const uint8_t page, const uint8_t column, const uint8_t* const buffer) {
    uint8_t translated = 0;

    // The MDOB128032GV uses the even rows and hence generates 4 pixels per byte written
    for (int pixel = 0; pixel < 4; pixel++) {

        // find the x-y location for this pixel
        // In current use, this display need to be reversed vertically and horizontally
        // TODO: consider making this an option.
        uint8_t x = (MDOB128032GV_COLS - 1) - column;
        uint8_t y = (MDOB128032GV_ROWS - 1) - (page*4 + pixel);

        // Apply column offset
        x += MDOB128032GV_OFFSET;
        if (x >= MDOB128032GV_COLS) x-= MDOB128032GV_COLS;

        // find the bit for this pixel in the buffer
        int location = MDOB128032GV_COLS * y + x;
        int bit = (buffer[location>>3] >> (location&0x7)) &0x01;

        // set the bit for the appropriate pixel.
        translated |= bit << (1 + 2*pixel);
    }


    return translated;
}

ssd1306_display MDOB128032GV = {
    MDOB128032GV_ROWS,
    MDOB128032GV_COLS,
    &MDOB128032GV_translator,
    MDOB128032GV_init_buf};
