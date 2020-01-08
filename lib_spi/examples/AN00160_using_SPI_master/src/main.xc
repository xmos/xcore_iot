// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <spi.h>
#include <syscall.h>
#include <timer.h>
#include <print.h>
#include <platform.h>

/* These ports are used for the SPI master */
out buffered port:32   p_sclk  = on tile[0]: XS1_PORT_1I;
out port               p_ss[1] = on tile[0]: {XS1_PORT_1J};
in buffered port:32    p_miso  = on tile[0]: XS1_PORT_1K;
out buffered port:32   p_mosi  = on tile[0]: XS1_PORT_1L;

clock clk0 = on tile[0]: XS1_CLKBLK_1;
clock clk1 = on tile[0]: XS1_CLKBLK_2;

/* This application function sends some traffic as SPI master using
 * the synchronous interface. Since this is run in simulation
 * there is no slave, so the incoming data (stored in the 'val'
 * variable) will just be zero.
 */
void app(client spi_master_if spi)
{
    uint8_t val;
    printstrln("Sending SPI traffic");
    delay_microseconds(30);
    spi.begin_transaction(0, 100, SPI_MODE_0);
    val = spi.transfer8(0xab);
    val = spi.transfer32(0xcc);
    val = spi.transfer8(0xfe);
    spi.end_transaction(100);

    delay_microseconds(40);
    spi.begin_transaction(0, 100, SPI_MODE_0);
    val = spi.transfer8(0x22);
    spi.end_transaction(100);

    printstrln("Done.");
    _exit(0);
}

/* This application function sends some traffic as SPI master using
 * the asynchronous interface. Since this is run in simulation
 * there is no slave, so the incoming data (stored in the 'val'
 * variable) will just be zero.
 */
void async_app(client spi_master_async_if spi)
{
    uint8_t indata[10];
    uint8_t outdata[10];
    uint8_t * movable inbuf = indata;
    uint8_t * movable outbuf = outdata;

    printstrln("Sending SPI traffic (async)");
    delay_microseconds(30);

    // Fill the out buffer with data
    outbuf[0] = 0xab;
    outbuf[1] = 0xcc;
    outbuf[2] = 0; outbuf[3] = 0; outbuf[4] = 0;
    outbuf[5] = 0xfe;
    spi.begin_transaction(0, 100, SPI_MODE_0);

    // This call passes the buffers over to the SPI task, after
    // this the application cannot access the buffers until
    // the retrieve_transfer_buffers_8 function is called.
    spi.init_transfer_array_8(move(inbuf),
                              move(outbuf),
                              6);

    // Your application can do calculation here whilst the spi task
    // transfers the buffer.

    // A select will wait for an event. In this case the event we are waiting
    // for is the transfer_complete() notification event from the SPI task.
    select {
        case spi.transfer_complete():
           // Once the transfer is complete, we can retrieve the
           // buffers back into the inbuf and outbuf pointer variables
           spi.retrieve_transfer_buffers_8(inbuf, outbuf);
           break;
    }

    spi.end_transaction(100);


    delay_microseconds(40);
    spi.begin_transaction(0, 100, SPI_MODE_0);
    outbuf[0] = 0x22;
    spi.init_transfer_array_8(move(inbuf),
                              move(outbuf),
                              1);
    select {
        case spi.transfer_complete():
           spi.retrieve_transfer_buffers_8(inbuf, outbuf);
           break;
    }
    spi.end_transaction(100);

    printstrln("Done.");
    _exit(0);
}

int main(void) {
  interface spi_master_if i_spi[1];
  par {
    on tile[0]: app(i_spi[0]);
    on tile[0]: spi_master(i_spi, 1,
                           p_sclk, p_mosi, p_miso, p_ss, 1,
                           null);
  }
  return 0;
}

/* Uncomment the main below (and comment out the one above) to try the
   async version.

int main(void) {
  interface spi_master_async_if i_spi_async[1];
  par {
    on tile[0]: async_app(i_spi_async[0]);
    on tile[0]: spi_master_async(i_spi_async, 1,
                                 p_sclk, p_mosi, p_miso, p_ss, 1,
                                 clk0, clk1);
  }
  return 0;
}

*/
