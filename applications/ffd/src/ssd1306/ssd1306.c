//// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "ssd1306.h"

// Include known displays
#include "MDOB128032GV.h"

void ssd1306_init(
        void* app_ctx,
        ssd1306_context* const ctx,
        const ssd1306_transport* const transport,
        const ssd1306_display_type type) {

    // Setup the list of displays
    ssd1306_display* display_types[ssd1306_END];
    display_types[ssd1306_MDOB128032GV] = &MDOB128032GV;
    // Add any new display type here

    //setup the context
    ctx->transport = transport;

    // TODO: add error checking
    ctx->display = display_types[type];


    int i=0;
    ssd1306_init_command send_buf;

    while (ctx->display->initialisation[i][0] != SSD1306_END_OF_LIST) {

        // The actual command follows the SEND_COMMAND value
        // TODO: consider if this should be in the transport layer
        send_buf[0] = SSD1306_SEND_COMMAND;

        for (int j = 1; j <= SSD1306_MAX_COMMAND_LENGTH; j++) {
            send_buf[j] = ctx->display->initialisation[i][j];
        }

        transport->write(
            app_ctx,
            ctx->transport->bus,
            ctx->transport->address,
            send_buf,
            ctx->display->initialisation[i][0] + 1);
        i++;
    }
}


void ssd1306_write(void* app_ctx, const ssd1306_context* const ctx, const uint8_t* const buf) {
    uint8_t send_buf[SSD1306_COLUMNS + 1];

    for (int page=0; page<SSD1306_PAGES; page++) {

    	// Write the page number which we are going to send data to
    	send_buf[0] = SSD1306_SEND_COMMAND;
    	send_buf[1] = SSD1306_SET_PAGE + page;
        ctx->transport->write(app_ctx, ctx->transport->bus, ctx->transport->address, send_buf, 2);

        // Setup data write
    	send_buf[0] = SSD1306_SEND_DATA;

        // Fill the rest of the buffer with data
    	for (int col=0;col<SSD1306_COLUMNS;col++)
            send_buf[col + 1] = ctx->display->translator(page, col, buf);

        // Write the data
        ctx->transport->write(app_ctx, ctx->transport->bus, ctx->transport->address, send_buf, SSD1306_COLUMNS + 1);
    }
}
