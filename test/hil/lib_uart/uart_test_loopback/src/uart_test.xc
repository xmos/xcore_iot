// Copyright (c) 2014-2016, XMOS Ltd, All rights reserved

#include <xs1.h>
#include <platform.h>
#include <stdlib.h>
#include "debug_print.h"
#include "xassert.h"
#include "uart.h"

#define BITTIME(x) (100000000 / (x))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define CHECK_EVENTS 1
#define CHECK_BUFFERING 1
#define CHECK_RUNTIME_PARAMETER_CHANGE 1
#define CHECK_PARITY_ERRORS 1

static void check_no_rx_data(client uart_rx_if i_uart_rx,
                             unsigned timeout,
                             int &result)
{
  timer t;
  unsigned time;
  t :> time;
  select {
  case t when timerafter(time + timeout) :> void:
    return;
  case i_uart_rx.data_ready():
    printhexln(i_uart_rx.read());
    result = 0;
    return;
  }
  __builtin_unreachable();
}

static void uart_test(client uart_tx_if i_uart_tx,
                      client uart_config_if i_tx_config,
                      client uart_rx_if i_uart_rx,
                      client uart_config_if i_rx_config,
                      unsigned baud_rate)
{
  unsigned char byte;
  timer t;
  unsigned time;

  debug_printf("TEST CONFIG:{'baud rate':%d}\n",baud_rate);

  // Configure RX, TX
  i_rx_config.set_baud_rate(baud_rate);
  i_tx_config.set_baud_rate(baud_rate);

  int result = 1;
  debug_printf("Performing basic loopback test.\n");
  i_uart_tx.write(0x0);

  byte = i_uart_rx.wait_for_data_and_read();
  result &= (byte == 0x0);

  i_uart_tx.write(0x2c);
  byte = i_uart_rx.wait_for_data_and_read();
  result &= (byte == 0x2c);

  i_uart_tx.write(0x0A);
  byte = i_uart_rx.wait_for_data_and_read();
  result &= (byte == 0x0A);

  i_uart_tx.write(0x04);
  byte = i_uart_rx.wait_for_data_and_read();
  result &= (byte == 0x04);

  debug_printf("TEST RESULT:basic_loopback:%s\n", result ? "PASS" : "FAIL");

  if (CHECK_EVENTS) {
    debug_printf("Check we can event on incoming data.\n");
    result = 1;
    t :> time;
    i_uart_tx.write(173);
    select {
    case t when timerafter(time + BITTIME(baud_rate) * 20) :> void:
      // Shouldn't get here.
      debug_printf("timeout (data not ready)");
      result = 0;
      break;
    case i_uart_rx.data_ready():
      byte = i_uart_rx.read();
      result &= (byte == 173);
      break;
    }
    debug_printf("TEST RESULT:rx_notification:%s\n", result ? "PASS" : "FAIL");
  }

  if (CHECK_BUFFERING) {
    result = 1;
    unsigned char data[] = { 0b10111111, 0x55, 0x2c, 0x09};
    debug_printf("Testing buffering of %d bytes.\n", ARRAY_SIZE(data));
    for (unsigned i = 0; i < ARRAY_SIZE(data); i++) {
      i_uart_tx.write(data[i]);
    }
    t :> time;
    t when timerafter(time + BITTIME(baud_rate) * 11 * 4) :> void;
    for (unsigned i = 0; i < ARRAY_SIZE(data); i++) {
      byte = i_uart_rx.wait_for_data_and_read();
      result &= (byte == data[i]);
    }
    debug_printf("TEST RESULT:rx_buffer:%s\n", result ? "PASS" : "FAIL");
  }

  if (CHECK_RUNTIME_PARAMETER_CHANGE) {
    // Reconfigure
    debug_printf("Reconfiguring UART.\n");
    result = 1;
    i_rx_config.set_baud_rate(baud_rate/2);
    i_rx_config.set_stop_bits(10);
    i_rx_config.set_bits_per_byte(7);
    i_tx_config.set_baud_rate(baud_rate/2);
    i_tx_config.set_stop_bits(10);
    i_tx_config.set_bits_per_byte(7);

    i_uart_tx.write(0x12);
    i_uart_tx.write(0x5a);
    byte = i_uart_rx.wait_for_data_and_read();
    result &= (byte == 0x12);
    byte = i_uart_rx.wait_for_data_and_read();
    result &= (byte == 0x5a);

    debug_printf("TEST RESULT:reconfiguration:%s\n", result ? "PASS" : "FAIL");

    debug_printf("Reconfiguring parity to odd.\n");
    result = 1;
    // Check parity
    i_rx_config.set_bits_per_byte(8);
    i_tx_config.set_bits_per_byte(8);
    i_rx_config.set_parity(UART_PARITY_ODD);
    i_tx_config.set_parity(UART_PARITY_ODD);
    i_uart_tx.write(0x12);
    byte = i_uart_rx.wait_for_data_and_read();
    result &= (byte == 0x12);
    i_uart_tx.write(0xa4);
    byte = i_uart_rx.wait_for_data_and_read();
    result &= (byte == 0xa4);

    debug_printf("TEST RESULT:reconfigure_parity_odd:%s\n", result ? "PASS" : "FAIL");

    debug_printf("Reconfiguring parity to even.\n");
    result = 1;
    // Check parity
    i_rx_config.set_parity(UART_PARITY_EVEN);
    i_tx_config.set_parity(UART_PARITY_EVEN);
    i_uart_tx.write(0x12);
    byte = i_uart_rx.wait_for_data_and_read();
    result &= (byte == 0x12);
    i_uart_tx.write(0xa4);
    byte = i_uart_rx.wait_for_data_and_read();
    result &= (byte == 0xa4);

    debug_printf("TEST RESULT:reconfigure_parity_even:%s\n", result ? "PASS" : "FAIL");

    debug_printf("Reconfiguring back ..\n");
    i_rx_config.set_baud_rate(baud_rate);
    i_rx_config.set_stop_bits(1);
    i_rx_config.set_bits_per_byte(8);
    i_rx_config.set_parity(UART_PARITY_NONE);
    i_tx_config.set_baud_rate(baud_rate);
    i_tx_config.set_stop_bits(1);
    i_tx_config.set_bits_per_byte(8);
    i_tx_config.set_parity(UART_PARITY_NONE);
  }


  if (CHECK_PARITY_ERRORS) {
    debug_printf("Checking that invalid parity information is discarded.\n");
    result = 1;
    i_rx_config.set_parity(UART_PARITY_ODD);
    i_tx_config.set_parity(UART_PARITY_EVEN);
    delay_ticks(1000);
    i_uart_tx.write(0x55);
    check_no_rx_data(i_uart_rx, BITTIME(baud_rate) * 200, result);

    i_rx_config.set_parity(UART_PARITY_EVEN);
    i_tx_config.set_parity(UART_PARITY_ODD);
    delay_ticks(1000);
    i_uart_tx.write(0x55);
    check_no_rx_data(i_uart_rx, BITTIME(baud_rate) * 200, result);
    debug_printf("TEST RESULT:parity_errors:%s\n", result ? "PASS" : "FAIL");

  }
  _Exit(0);
}

port p_rx = on tile[0] : XS1_PORT_1A;
port p_tx = on tile[1] : XS1_PORT_1B;


#define BUFFER_SIZE 64
int main() {
  uart_rx_if i_rx;
  uart_tx_if i_tx;
  uart_config_if i_rx_config, i_tx_config;
  input_gpio_if i_gpio_rx;
  output_gpio_if i_gpio_tx[1];
  par {

    on tile[1] : output_gpio(i_gpio_tx, 1, p_tx, null);
    on tile[1] : uart_tx(i_tx, i_tx_config,
                         115200, UART_PARITY_NONE, 8, 1, i_gpio_tx[0]);
    on tile[0].core[0] : input_gpio_1bit_with_events(i_gpio_rx, p_rx);
    on tile[0].core[0] : uart_rx(i_rx, i_rx_config, BUFFER_SIZE,
                                 115200, UART_PARITY_NONE, 8, 1, i_gpio_rx);
    on tile[0] : {
      #if SMOKE_TEST
      unsigned rates[] = {115200};
      #else
      unsigned rates[] = {2400, 9600, 19200};
      #endif
      for (int i = 0; i < ARRAY_SIZE(rates); i++) {
        uart_test(i_tx, i_tx_config, i_rx, i_rx_config, rates[i]);
      }
      _Exit(0);
     }
    par (int i=0;i<6;i++)
    on tile[0]: while(1);
   }
   return 0;
 }
