# Copyright 2014-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import xmostest
from i2c_slave_checker import I2CSlaveChecker
import os


def do_slave_test(speed, level):
    resources = xmostest.request_resource("xsim")

    binary = 'i2c_slave_test/bin/i2c_slave_test.xe'

    checker = I2CSlaveChecker("tile[0]:XS1_PORT_1A",
                              "tile[0]:XS1_PORT_1B",
                              tsequence =
                              [("w", 0x3c, [0x33, 0x44, 0x3]),
                               ("r", 0x3c, 3),
                               ("w", 0x3c, [0x99]),
                               ("w", 0x44, [0x33]),
                               ("r", 0x3c, 1),
                               ("w", 0x3c, [0x22, 0xff])],
                               speed = speed)

    tester = xmostest.ComparisonTester(open('expected/basic_slave_test.expect'),
                                     'lib_i2c', 'i2c_slave_sim_tests',
                                     'basic_test', {'speed':speed},
                                     regexp=True)

    tester.set_min_testlevel(level)

    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads = [checker],
                              simargs=['--weak-external-drive'],
                              suppress_multidrive_messages = True,
                              tester = tester)

def runtest():
    do_slave_test(400, 'smoke')
    do_slave_test(100, 'nightly')
    do_slave_test(10, 'nightly')
