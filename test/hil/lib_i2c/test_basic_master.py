# Copyright 2014-2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
import pytest
from pathlib import Path
from i2c_master_checker import I2CMasterChecker

speed_args = {"400kbps": 400,
              "100kbps": 100,
              "10kbps": 10}

port_setup_args = {"SCL:1b,SDA:1b": 0,
                   "SCL:8b,SDA:8b,shared": 1,
                   "SCL:8b,SDA:8b": 2,
                   "SCL:1b,SDA:8b,overlap": 3,
                   "SCL:8b,SDA:1b,overlap": 4}

stop_args = {"stop": "stop",
             "no_stop": "no_stop"}

@pytest.mark.parametrize("port_setup", port_setup_args.values(), ids=port_setup_args.keys())
@pytest.mark.parametrize("stop", stop_args.values(), ids=stop_args.keys())
@pytest.mark.parametrize("speed", speed_args.values(), ids=speed_args.keys())
# capfd here is an inbuilt test fixture allowing access to stdout and stderr
def test_i2c_basic_master(build, capfd, request, nightly, stop, speed, port_setup):
    if (speed == 10) and not nightly:
        pytest.skip("10kbps only tested with --nightly option")

    cwd = Path(request.fspath).parent

    id_string = f"{speed}_{stop}_{port_setup}"
    # It is assumed that this is of the form <arbitrary>/bin/<unique>/.../<executable>.xe,
    # and that <arbitrary> contains the CMakeLists.txt file for all test executables.
    binary = f'{cwd}/i2c_master_test/bin/test_hil_i2c_master_test_{id_string}.xe'

    port_map = [["tile[0]:XS1_PORT_1A", "tile[0]:XS1_PORT_1B"],     # Test 1b port SCL 1b port SDA
                ["tile[0]:XS1_PORT_8A.1", "tile[0]:XS1_PORT_8A.3"], # Test 8b port shared by SCL and SDA
                ["tile[0]:XS1_PORT_8A", "tile[0]:XS1_PORT_8B"],     # Test 8b port SCL 8b port SDA
                ["tile[0]:XS1_PORT_1M", "tile[0]:XS1_PORT_8D.1"],   # Test 1b port SCL with overlapping 8b port SDA
                ["tile[0]:XS1_PORT_8D.1", "tile[0]:XS1_PORT_1M"]]   # Test 8b port SCL with overlapping 1b port SDA

    checker = I2CMasterChecker(port_map[port_setup][0],
                            port_map[port_setup][1],
                            tx_data = [0x99, 0x3A, 0xff],
                            expected_speed = speed,
                            ack_sequence=[True, True, False,
                                            True,
                                            True,
                                            True, True, True, False,
                                            True, False])

    tester = px.testers.PytestComparisonTester(f'{cwd}/expected/master_test_{stop}.expect',
                                            regexp = True,
                                            ordered = True)

    sim_args = ['--weak-external-drive']

    # The environment here should be set up with variables defined in the
    # CMakeLists.txt file to define the build


    ## Temporarily building externally, see hil/build_lib_i2c_tests.sh
    # build(directory = binary,
    #         env = {"PORT_SETUPS":port_setup, "SPEEDS":speed, "STOPS":stop},
    #         bin_child = id_string)

    px.run_with_pyxsim(binary,
                    simthreads = [checker],
                    simargs = sim_args)

    tester.run(capfd.readouterr().out)
