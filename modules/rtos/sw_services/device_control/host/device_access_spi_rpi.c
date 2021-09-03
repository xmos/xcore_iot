// Copyright 2017-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#if USE_SPI && RPI

#include <stdio.h>
#include <unistd.h>
#include "control.h"
#include "device_control_host.h"
#include "control_host_support.h"
#include "bcm2835.h"

//#define DBG(x) x
#define DBG(x)

static unsigned delay_milliseconds;

static bcm2835SPIMode lib_spi_to_bcm2835_mode(spi_mode_t spi_mode)
{
  if(spi_mode == SPI_MODE_0) return BCM2835_SPI_MODE1;
  else if(spi_mode == SPI_MODE_1) return BCM2835_SPI_MODE0;
  else if(spi_mode == SPI_MODE_2) return BCM2835_SPI_MODE2;
  else return BCM2835_SPI_MODE3;
}

control_ret_t
control_init_spi_pi(spi_mode_t spi_mode, bcm2835SPIClockDivider clock_divider, unsigned delay_for_read)
{
  if(!bcm2835_init() ||
     !bcm2835_spi_begin()) {
    fprintf(stderr, "bcm2835 initialisation failed. Possibly not running as root\n");
    return CONTROL_ERROR;
  }

  delay_milliseconds = delay_for_read;

  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(lib_spi_to_bcm2835_mode(spi_mode));
  bcm2835_spi_setClockDivider(clock_divider);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	
  return CONTROL_SUCCESS;
}

control_ret_t
control_write_command(control_resid_t resid, control_cmd_t cmd,
                      const uint8_t payload[], size_t payload_len)
{
  uint8_t data_sent_recieved[SPI_TRANSACTION_MAX_BYTES];

  int data_len = control_build_spi_data(data_sent_recieved, resid, cmd, payload, payload_len);
  bcm2835_spi_transfern((char *)data_sent_recieved, data_len);

  return CONTROL_SUCCESS;
}

control_ret_t
control_read_command(control_resid_t resid, control_cmd_t cmd,
                     uint8_t payload[], size_t payload_len)
{
  uint8_t data_sent_recieved[SPI_TRANSACTION_MAX_BYTES] = {0};
  int data_len = control_build_spi_data(data_sent_recieved, resid, cmd, payload, payload_len);

  bcm2835_spi_transfern((char *)data_sent_recieved, data_len);
  
  usleep(delay_milliseconds * 1000);
  memset(data_sent_recieved, 0, SPI_TRANSACTION_MAX_BYTES);
  unsigned transaction_length = payload_len < 8 ? 8 : payload_len;  

  bcm2835_spi_transfern((char *)data_sent_recieved, transaction_length);
  DBG(printf("Data recieved: "));
  for(unsigned i=0; i<transaction_length; ++i) {
    DBG(printf("%-3d ", (uint8_t) data_sent_recieved[i]));
  }

  memcpy(payload, data_sent_recieved, payload_len);

  return CONTROL_SUCCESS;
}

control_ret_t
control_cleanup_spi(void)
{
  bcm2835_spi_end();
  bcm2835_close();
  return CONTROL_SUCCESS;
}

#endif /* USE_SPI && RPI */
