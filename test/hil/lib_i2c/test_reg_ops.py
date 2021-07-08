# Copyright 2014-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import xmostest
from i2c_master_checker import I2CMasterChecker
import os

def do_test():
    resources = xmostest.request_resource("xsim")

    binary = 'i2c_master_reg_test/bin/i2c_master_reg_test.xe'

    checker = I2CMasterChecker("tile[0]:XS1_PORT_1A",
                               "tile[0]:XS1_PORT_1B",
                               tx_data = [0x99, 0x3A, 0xff, 0x05, 0xee, 0x06],
                               expected_speed = 400,
                               ack_sequence=[True, True, False,
                                             True, True, True, False,
                                             True, True, True, True, False,
                                             True, True, True, False,
                                             True, True,
                                             True, True, True,
                                             True, True, True, True,
                                             True, True, True])

    tester = xmostest.ComparisonTester(open('expected/reg_test.expect'),
                                       'lib_i2c', 'i2c_master_sim_tests',
                                       'reg_ops_test',
                                       {},
                                       regexp=True)

    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads=[checker],
                              simargs=['--weak-external-drive'],
                              suppress_multidrive_messages=True,
                              tester=tester)

def runtest():
    do_test()
