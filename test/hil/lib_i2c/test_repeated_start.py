# Copyright 2014-2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
from pathlib import Path
from i2c_master_checker import I2CMasterChecker

def test_i2c_repeated_start(build, capfd, request):
    cwd = Path(request.fspath).parent
    binary = f'{cwd}/i2c_test_repeated_start/bin/test_hil_i2c_test_repeated_start.xe'

    checker = I2CMasterChecker("tile[0]:XS1_PORT_1A",
                               "tile[0]:XS1_PORT_1B",
                               expected_speed=400)

    tester = px.testers.PytestComparisonTester(f'{cwd}/expected/repeated_start.expect',
                                                regexp = True,
                                                ordered = True)

    sim_args = ['--weak-external-drive']

    ## Temporarily building externally, see hil/build_lib_i2c_tests.sh
    # build(binary)

    px.run_with_pyxsim(binary,
                    simthreads = [checker],
                    simargs = sim_args)

    tester.run(capfd.readouterr().out)
