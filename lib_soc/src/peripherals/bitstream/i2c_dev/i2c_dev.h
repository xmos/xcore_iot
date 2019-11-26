// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef I2C_DEV_H_
#define I2C_DEV_H_

#if __rtos_peripherals_conf_h_exists__
#include "rtos_peripherals_conf.h"
#endif

#include "i2c_dev_conf_defaults.h"

#include "i2c.h"
#include "i2c_dev_ctrl.h"

[[combinable]]
void i2c_dev(
        chanend ctrl_c,
        client i2c_master_if i2c);

#endif /* I2C_DEV_H_ */
