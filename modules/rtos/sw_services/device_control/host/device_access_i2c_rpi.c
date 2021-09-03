// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#if USE_I2C && RPI

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "device_control_host.h"
#include "control_host_support.h"
#include "util.h"

//#define DBG(x) x
#define DBG(x)

/*Note there is an issue with RPI/Jessie where I2C repeated starts are not enabled by default.
Try the following at the bash command line to enable them:

sudo su -
echo -n 1 > /sys/module/i2c_bcm2708/parameters/combined
exit
*/

const char *devName = "/dev/i2c-1";                // Name of the i2c device we will be using
unsigned char address = 0xff;                // Slave address. Initialise to invalid
int fd;                                      // File descrition for i2c device

control_ret_t control_init_i2c(unsigned char i2c_slave_address)
{
  // Previously this shifted the address down by 1(>>1)
  // but this wasn't found to be necessary
  address = i2c_slave_address;

  if ((fd = open(devName, O_RDWR)) < 0) {          // Open port for reading and writing
    fprintf(stderr, "Failed to open i2c port: ");
    perror( "" );
    return CONTROL_ERROR;
  }
  
  if (ioctl(fd, I2C_SLAVE, address) < 0) {          // Set the port options and set the address of the device we wish to speak to
    fprintf(stderr, "Unable to set i2c configuration at address 0x%x: ", address);
    perror( "" );
    return CONTROL_ERROR;
  }

  DBG(printf("Configured to talk to i2c device at address 0x%x = (0x%x >> 1)\n", address, i2c_slave_address));

  // This writes command zero to register zero. It is a workaround for RPI kernel 4.4 which seems to ignore the first data bytes otherwise
  // It is a benign operation for lib_device_control as register zero, command zero is the version and is read only
  unsigned char data[3];
  control_build_i2c_data(data, 0, 0, data, 0);
  write(fd, data, 3);

  return CONTROL_SUCCESS;
}

static unsigned num_commands = 0;

control_ret_t
control_write_command(control_resid_t resid, control_cmd_t cmd,
                      const uint8_t payload[], size_t payload_len)
{
  unsigned char buffer_to_send[I2C_TRANSACTION_MAX_BYTES + 3];
  int len = control_build_i2c_data(buffer_to_send, resid, cmd, payload, payload_len);

  DBG(printf("%u: send write command: ", num_commands));
  DBG(print_bytes((unsigned char*)buffer_to_send, payload_len));
	
  int written = write(fd, buffer_to_send, len);
  if (written != len){
    fprintf(stderr, "Error writing to i2c. %d of %d bytes sent\n", written, len);
    return CONTROL_ERROR;
  }

  num_commands++;

  return CONTROL_SUCCESS;
}

control_ret_t
control_read_command(control_resid_t resid, control_cmd_t cmd,
                     uint8_t payload[], size_t payload_len)
{
  unsigned char read_hdr[I2C_TRANSACTION_MAX_BYTES];
  unsigned len = control_build_i2c_data(read_hdr, resid, cmd, payload, payload_len);
  if (len != 3){
    fprintf(stderr, "Error building read command section of read_device. len should be 3 but is %d\n", len);
    return CONTROL_ERROR;
  }

  // Do a repeated start (write followed by read with no stop bit)
  struct i2c_msg rdwr_msgs[2] = {
    {  // Start address
      .addr = address,
      .flags = 0, // write
      .len = (unsigned short)len, //will be 3
      .buf = read_hdr
    },
    { // Read buffer
      .addr = address,
      .flags = I2C_M_RD, // read
      .len = (unsigned short)payload_len,
      .buf = payload
    }
  };

  struct i2c_rdwr_ioctl_data rdwr_data = {
    .msgs = rdwr_msgs,
    .nmsgs = 2
  };

  DBG(printf("%d: issued command to read %d bytes: command=", num_commands, payload_len));
  DBG(print_bytes((unsigned char*)read_hdr, len));

  int errno = ioctl( fd, I2C_RDWR, &rdwr_data );

  if ( errno < 0 ) {
    fprintf(stderr, "rdwr ioctl error %d: ", errno );
    perror( "" );
    return CONTROL_ERROR;
  }

  DBG(printf("read command received: "));
  DBG(print_bytes(payload, payload_len));

  num_commands++;

  return CONTROL_SUCCESS;
}

control_ret_t control_cleanup_i2c(void)
{
  close(fd);
  return CONTROL_SUCCESS;
}

#endif // USE_I2C
