#!/usr/bin/env python2
# Copyright 2014-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import xmostest
from i2c_master_checker import I2CMasterChecker

if __name__ == "__main__":
    xmostest.init()

    xmostest.register_group("lib_i2c",
                            "i2c_master_sim_tests",
                            "I2C master simulator tests",
    """
Tests are performed by running the I2C library connected to a
simulator model (written as a python plugin to xsim). The simulator
model checks that the signals comply to the I2C specification and checks the
protocol speed of the transactions. Tests are run to test the following
features:

    * Transmission of packets
    * Reception of packets

The tests are run with transactions of varying number of bytes and with rx and
tx transactions interleaved. The tests are run at speeds of 10, 100 and 400
Kbps.
""")

    xmostest.register_group("lib_i2c",
                            "i2c_slave_sim_tests",
                            "I2C slave simulator tests",
    """
Tests are performed by running the I2C library connected to a
simulator model (written as a python plugin to xsim). The simulator
model checks that the signals comply to the I2C specification and checks the
protocol speed of the transactions. Tests are run to test the following
features:

    * Transmission of packets
    * Reception of packets

The tests are run with transactions of varying number of bytes and with rx and
tx transactions interleaved. The tests are run at speeds of 10, 100 and 400
Kbps.
""")

    xmostest.runtests()

    xmostest.finish()
