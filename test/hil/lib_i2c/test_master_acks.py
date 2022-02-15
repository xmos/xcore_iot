# Copyright 2014-2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
import pytest
from pathlib import Path
from i2c_master_checker import I2CMasterChecker

stop_args = {"stop": "stop",
             "no_stop": "no_stop"}

@pytest.mark.parametrize("stop", stop_args.values(), ids=stop_args.keys())
def test_i2c_master_acks(build, capfd, request, stop):

    # It is assumed that this is of the form <arbitrary>/bin/<unique>/.../<executable>.xe,
    # and that <arbitrary> contains the CMakeLists.txt file for all test executables.
    cwd = Path(request.fspath).parent
    binary = f'{cwd}/i2c_master_test/bin/test_hil_i2c_master_test_tx_only_{stop}.xe'

    checker = I2CMasterChecker("tile[0]:XS1_PORT_1A",
                               "tile[0]:XS1_PORT_1B",
                               tx_data = [0x99, 0x3A, 0xff],
                               expected_speed = 400,
                               ack_sequence=[True, True, True,
                                             True, True, False,
                                             False, True])

    tester = px.testers.PytestComparisonTester(f'{cwd}/expected/ack_test_{stop}.expect',
                                                regexp = True,
                                                ordered = True)

    sim_args = ['--weak-external-drive']

    # The environment here should be set up with variables defined in the
    # CMakeLists.txt file to define the build


    ## Temporarily building externally, see hil/build_lib_i2c_tests.sh
    # build(directory = binary,
    #         env = {"STOPS":stop, "ACK_TEST":True},
    #         bin_child = f"ack_{stop}")

    px.run_with_pyxsim(binary,
                    simthreads = [checker],
                    simargs = sim_args)

    tester.run(capfd.readouterr().out)
