# Copyright 2014-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
from i2c_master_checker import I2CMasterChecker

def test_bus_lock(build, capfd):
    binary = 'i2c_test_locks/bin/i2c_test_locks.xe' 

    checker = I2CMasterChecker("tile[0]:XS1_PORT_1A",
                               "tile[0]:XS1_PORT_1B",
                               expected_speed = 400)

    tester = px.testers.PytestComparisonTester('expected/lock_test.expect',
                                                regexp = True,
                                                ordered = True)

    sim_args = ['--weak-external-drive']

    build(binary)

    px.run_with_pyxsim(binary,
                    simthreads = [checker],
                    simargs = sim_args)
                    
    tester.run(capfd.readouterr().out)
