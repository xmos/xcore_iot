# Copyright 2014-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import xmostest
from i2c_master_checker import I2CMasterChecker
import os

def do_test():
    resources = xmostest.request_resource("xsim")

    binary = 'i2c_test_repeated_start/bin/i2c_test_repeated_start.xe'

    checker = I2CMasterChecker("tile[0]:XS1_PORT_1A",
                               "tile[0]:XS1_PORT_1B",
                               expected_speed=400)

    sim_args = ['--weak-external-drive']

    tester = xmostest.ComparisonTester(open('expected/repeated_start.expect'),
                                      'lib_i2c', 'i2c_master_sim_tests',
                                      'repeated_start',
                                      {},
                                      regexp=True)

    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads = [checker],
                              simargs=sim_args,
                              suppress_multidrive_messages = True,
                              tester = tester)

def runtest():
    do_test()
