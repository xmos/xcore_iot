# Copyright 2016-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import pytest
import Pyxsim as px
import subprocess
from pathlib import Path

sample_rate_args = {"768kbps": 768000,
                    "384kbps": 384000,
                    "192kbps": 192000}

num_channels_args = {"1ch": 1,
                     "2ch": 2,
                     "3ch": 3,
                     "4ch": 4}

rx_tx_inc_args = {"rx_delay_inc_50ns,tx_delay_inc_50ns": (5, 5),
                  "rx_delay_inc_0ns,tx_delay_inc_100ns": (0, 10),
                  "rx_delay_inc_100ns,tx_delay_inc_0ns": (10, 0)}

@pytest.mark.parametrize("sample_rate", sample_rate_args.values(), ids=sample_rate_args.keys())
@pytest.mark.parametrize("num_channels", num_channels_args.values(), ids=num_channels_args.keys())
@pytest.mark.parametrize(("receive_increment", "send_increment"), rx_tx_inc_args.values(), ids=rx_tx_inc_args.keys())
def test_i2s_backpressure(build, nightly, capfd, request, sample_rate, num_channels, receive_increment, send_increment):
    if (num_channels != 4) and not nightly:
        pytest.skip("Only run 4 channel tests unless it is a nightly")

    id_string = f"{sample_rate}_{num_channels}_{receive_increment}_{send_increment}"

    cwd = Path(request.fspath).parent

    binary = f'{cwd}/backpressure_test/bin/test_hil_backpressure_test_{id_string}.xe'

    tester = px.testers.PytestComparisonTester(f'{cwd}/expected/backpressure_test.expect',
                                            regexp = True,
                                            ordered = True)

    ## Temporarily building externally, see hil/build_lib_i2s_tests.sh
    # build(directory = binary,
    #         env = {"SAMPLE_RATES":sample_rate, "CHANS":num_channels, "RX_TX_INCS":f"{receive_increment};{send_increment}"},
    #         bin_child = id_string)

    subprocess.run(("xsim", binary, "--plugin", "LoopbackPort.dll", "-port tile[0] XS1_PORT_1G 1 0 -port tile[0] XS1_PORT_1A 1 0"))

    tester.run(capfd.readouterr().out)
