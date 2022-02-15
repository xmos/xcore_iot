# Copyright 2014-2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
import pytest
from pathlib import Path
from i2c_slave_checker import I2CSlaveChecker

speed_args = {"400kbps": 400,
              "100kbps": 100,
              "10kbps": 10}

@pytest.mark.parametrize("speed", speed_args.values(), ids=speed_args.keys())
def test_i2c_basic_slave(build, capfd, request, nightly, speed):
    if (speed != 400) and not nightly:
        pytest.skip("Speeds other than 400kbps only tested with option --nightly")

    cwd = Path(request.fspath).parent

    binary = f'{cwd}/i2c_slave_test/bin/test_hil_i2c_slave_test.xe'

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

    tester = px.testers.PytestComparisonTester(f'{cwd}/expected/basic_slave_test.expect',
                                            regexp = True,
                                            ordered = True)

    sim_args = ['--weak-external-drive']

    # The environment here should be set up with variables defined in the
    # CMakeLists.txt file to define the build. For this test, speed is only
    # used in the Python harness, not in the resultant xe, therefore it is
    # not passed to the build system.


    ## Temporarily building externally, see hil/build_lib_i2c_tests.sh
    # build(directory = binary,
    #         bin_child = f"{speed}")

    px.run_with_pyxsim(binary,
                    simthreads = [checker],
                    simargs = sim_args)

    tester.run(capfd.readouterr().out)
