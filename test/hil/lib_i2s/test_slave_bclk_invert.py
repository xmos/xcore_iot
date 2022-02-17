# Copyright 2015-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
from i2s_slave_checker import I2SSlaveChecker
from i2s_master_checker import Clock
from pathlib import Path
import pytest
import Pyxsim as px

num_in_out_args = {"2ch_in,2ch_out": (2, 2)}

@pytest.mark.parametrize(("num_in", "num_out"), num_in_out_args.values(), ids=num_in_out_args.keys())
def test_i2s_slave_bclk_invert(build, capfd, nightly, request, num_in, num_out):
    test_level = "0" if nightly else "1"
    id_string = f"{test_level}_{num_in}_{num_out}"
    cwd = Path(request.fspath).parent
    binary = f'{cwd}/i2s_slave_test/bin/test_hil_i2s_slave_test_{id_string}_inv.xe'

    clk = Clock("tile[0]:XS1_PORT_1A")

    checker = I2SSlaveChecker(
        "tile[0]:XS1_PORT_1B",
        "tile[0]:XS1_PORT_1C",
        ["tile[0]:XS1_PORT_1H","tile[0]:XS1_PORT_1I","tile[0]:XS1_PORT_1J", "tile[0]:XS1_PORT_1K"],
        ["tile[0]:XS1_PORT_1D","tile[0]:XS1_PORT_1E","tile[0]:XS1_PORT_1F", "tile[0]:XS1_PORT_1G"],
        "tile[0]:XS1_PORT_1L",
        "tile[0]:XS1_PORT_16A",
        "tile[0]:XS1_PORT_1M",
         clk,
         invert_bclk = True)

    tester = px.testers.PytestComparisonTester(f'{cwd}/expected/bclk_invert.expect',
                                            regexp = True,
                                            ordered = True,
                                            ignore = ["CONFIG:.*?"])

    ## Temporarily building externally, see hil/build_lib_i2s_tests.sh
    # build(directory = binary,
    #         env = {"NUMS_IN_OUT":f'{num_in};{num_out}', "TEST_LEVEL":f'{test_level}', "INVERT":"1"},
    #         bin_child = f"{id_string}_inv")

    px.run_with_pyxsim(binary,
                        simthreads = [clk, checker])

    tester.run(capfd.readouterr().out)
