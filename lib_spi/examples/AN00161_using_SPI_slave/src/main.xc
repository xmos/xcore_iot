// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <spi.h>
#include <stdint.h>
#include <timer.h>
#include <debug_print.h>
#include <platform.h>

/* These ports are used for the SPI slave task */
in port                 p_sclk = on tile[0]: XS1_PORT_1E;
in port                 p_ss   = on tile[0]: XS1_PORT_1F;
out buffered port:32    p_miso = on tile[0]: XS1_PORT_1G;
in buffered port:32     p_mosi = on tile[0]: XS1_PORT_1H;
clock                   cb     = on tile[0]: XS1_CLKBLK_1;

/* These ports are used for the SPI master task which is
   used to test the SPI slave (via simulator loopback). */
out buffered port:32   p_test_sclk  = on tile[0]: XS1_PORT_1I;
out port               p_test_ss[1] = on tile[0]: {XS1_PORT_1J};
in buffered port:32    p_test_miso  = on tile[0]: XS1_PORT_1K;
out buffered port:32   p_test_mosi  = on tile[0]: XS1_PORT_1L;

/* Interface to communicate between the register file tasks
   and the application. */
typedef interface reg_if {
  uint8_t get_reg(uint8_t regnum);
  void set_reg(uint8_t regnum, uint8_t value);
} reg_if;

#define NUM_REG 5

enum reg_state_t {
  WRITE_REG = 0,
  READ_REG = 1,
  WRITE_REG_DATA,
  READ_REG_DATA,
  IDLE
};

/* This application implements a register file which can be accessed by
 * the application on chip as well as over the SPI bus.
 *
 * To do this, the reg_file task reacts to both the SPI slave component or
 * requests from the application.
 *
 * Over SPI, the master is expected to assert the slave select line and
 * then send either a 0 for writing a register or a 1 for reading a register in
 * the first byte. The next byte is expected to contain the register number
 * to read/write. The final byte either receives or transmits the register
 * value.
 */
[[distributable]]
void reg_file(server spi_slave_callback_if i_spi,
              server reg_if i_reg)
{
  /* This array holds the register values */
  uint8_t reg_data[NUM_REG] = {0};

  /* This variable holds the current state of the register file with respect
   * to the SPI bus (i.e. what stage of the transaction over SPI it is at).
   */
  enum reg_state_t state = IDLE;

  /* This variable holds the current register being addressed over the SPI
   * bus.
   */
  uint8_t addr = 0;

  while (1) {
      select {
      /* These cases react to the SPI slave bus. A write from the bus will
       * update the state of the transaction. A read from the bus will get
       * sent the data from the currently addressed register. */
      case i_spi.master_ends_transaction(void):
          state = IDLE;
          break;
      case i_spi.master_requires_data(void) -> uint32_t data:
          data = reg_data[addr];
          break;
      case i_spi.master_supplied_data(uint32_t datum, uint32_t valid_bits):
          switch (state) {
          case IDLE:
              if (datum == WRITE_REG || datum == READ_REG)
                  state = datum;
              break;
          case READ_REG:
              if (datum < NUM_REG) {
                  addr = datum;
                  state = READ_REG_DATA;
              } else {
                  state = IDLE;
              }
              break;
          case READ_REG_DATA:
              // Do nothing with master data during a read data operation.
              break;
          case WRITE_REG:
              if (datum < NUM_REG) {
                  addr = datum;
                  state = WRITE_REG_DATA;
              } else {
                  state = IDLE;
              }
              break;
          case WRITE_REG_DATA:
              reg_data[addr] = datum;
              break;
          }
          break;

      /* The following cases respond to the application when it
       * requests to get/set a register.
       */
      case i_reg.get_reg(uint8_t regnum) -> uint8_t value:
          value = reg_data[regnum];
          break;
      case i_reg.set_reg(uint8_t regnum, uint8_t value):
          reg_data[regnum] = value;
          break;
      }
  }
}

/* This dummy application just sets a register value and then continually
 * polls and prints out another register value. It uses an interface connection
 * to the reg_file tasks to get/set registers.
 */
void app(client reg_if reg) {
    reg.set_reg(0, 0xfe);
    debug_printf("APP: Set register 1 to 0xFE\n");
    while (1) {
        delay_microseconds(20);
        debug_printf("APP: Register 1 is 0x%x\n", reg.get_reg(1));
    }
}

/* The tester task is not part of the application but just sends commands
 * over an SPI master bus on the same chip. The simulation can then loopback
 * the SPI ports onto the SPI slave bus to provide test traffic. This loopback
 * needs to be set up in the run configuration of the application.
 */
void tester(client spi_master_if spi)
{
    delay_microseconds(45);
    uint8_t val;
    spi.begin_transaction(0, 100, SPI_MODE_0);
    spi.transfer8(READ_REG); // READ command
    spi.transfer8(0); // REGISTER 0
    val = spi.transfer8(0); // DATA
    spi.end_transaction(100);
    debug_printf("SPI MASTER: Read register 0: 0x%x\n", val);

    spi.begin_transaction(0, 100, SPI_MODE_0);
    spi.transfer8(WRITE_REG); // WRITE command
    spi.transfer8(1); // REGISTER 1
    spi.transfer8(0xac); // DATA
    spi.end_transaction(100);
    debug_printf("SPI MASTER: Set register 1 to 0xAC\n");
}

int main(void) {
  interface spi_slave_callback_if i_spi;
  interface reg_if i_reg;
  interface spi_master_if i_spi_master[1];
  par {
    on tile[0]: spi_slave(i_spi, p_sclk, p_mosi, p_miso, p_ss, cb, SPI_MODE_0,
                          SPI_TRANSFER_SIZE_8);
    on tile[0]: reg_file(i_spi, i_reg);
    on tile[0]: app(i_reg);

    // These tasks are not part of the application but a test harness to
    // provide SPI master data which is expected to be looped back in
    // simulation to the SPI slave ports.
    on tile[0]: tester(i_spi_master[0]);
    on tile[0]: spi_master(i_spi_master, 1,
                           p_test_sclk, p_test_mosi, p_test_miso, p_test_ss,
                           1, null);
  }
  return 0;
}


