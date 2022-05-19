#!/usr/bin/env python
# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import subprocess
from pathlib import Path
import Pyxsim as px
import pytest

def test_fifo(request, capfd):
    cwd = Path(request.fspath).parent

    def dummy():
        pass
    tester = px.testers.PytestComparisonTester(f'{cwd}/expected/test_fifo_uart.expect',
                                            regexp = False,
                                            ordered = True,
                                            ignore = ["TEST CONFIG:.*"])

    binary = f'{cwd}/uart_test_fifo/bin/test_hil_uart_fifo_test.xe'

    px.run_with_pyxsim(binary, simthreads = [], simargs = ["--trace-to", "trace.txt"])
    capture = capfd.readouterr().out[:-1] #Tester appends an extra line feed which we don't need


    tester.run(capture)