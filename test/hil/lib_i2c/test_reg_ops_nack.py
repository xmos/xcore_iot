# Copyright 2014-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
from i2c_master_checker import I2CMasterChecker

def test_reg_ops(build, capfd):
    binary = 'i2c_master_reg_test/bin/i2c_master_reg_test.xe'

    checker = I2CMasterChecker("tile[0]:XS1_PORT_1A",
                               "tile[0]:XS1_PORT_1B",
                               tx_data = [0x99, 0x3A, 0xff, 0x05, 0xee, 0x06],
                               expected_speed = 400,
                               ack_sequence=[False, # NACK header
                                             True, True, False, # NACK before data
                                             True, False, # NACK before data
                                             True, False, # NACK before data
                                             False, # NACK address
                                             True, False, # NACK before data
                                             True, False, # NACK before data
                                             True, True, False # NACK before data
                                            ])

    tester = px.testers.PytestComparisonTester('expected/reg_ops_nack.expect',
                                                regexp = True,
                                                ordered = True)

    sim_args = ['--weak-external-drive']

    build(binary)

    px.run_with_pyxsim(binary,
                    simthreads = [checker],
                    simargs = sim_args)

    # The first two lines of this test are not reflected in the expectation file
    # and vary based on the test; cut them out.
    outcapture = capfd.readouterr().out.split("\n")[2:]

    tester.run(outcapture)

