#!/usr/bin/env python
# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

from uart_tx_checker import UARTTxChecker
from pathlib import Path
import Pyxsim as px
import pytest

buffered_args = {   "unbuffered" : 0,
                    "buffered": 1}

speed_args = {
              # "1843200 baud": 1843200,
              "921600 baud": 921600,
              "576000 baud": 576000,
              "115200 baud": 115200,
              "9600 baud": 9600
              }

data_bit_args = {   "eight": 8,            
                    "five": 5}

parity_args = { "NONE": 0,
                "EVEN": 1,
                "ODD": 2}                

stop_bit_args = {   "one": 1,
                    "two": 2}

# buffered_args = {   "unbuffered" : 0}
# speed_args = {"921600 baud": 921600}
# data_bit_args = {"eight": 8}
# parity_args = { "NONE": 0}                
# stop_bit_args = {"one": 1}

@pytest.mark.parametrize("buffered", buffered_args.values(), ids=buffered_args.keys())
@pytest.mark.parametrize("baud", speed_args.values(), ids=speed_args.keys())
@pytest.mark.parametrize("bpb", data_bit_args.values(), ids=data_bit_args.keys())
@pytest.mark.parametrize("parity", parity_args.values(), ids=parity_args.keys())
@pytest.mark.parametrize("stop", stop_bit_args.values(), ids=stop_bit_args.keys())
def test_uart_tx(request, capfd, buffered, baud, bpb, parity, stop):
    myenv = {'baud': baud}
    cwd = Path(request.fspath).parent
    parity_key = [key for key, value in parity_args.items() if value == parity][0] #reverse lookup because we use the parity key name in the binary
    binary = f'{cwd}/uart_test_tx/bin/test_hil_uart_tx_test_buffer{buffered}_{baud}_{bpb}_{parity_key}_{stop}.xe'

    length_of_test = 4
    tx_port = "tile[0]:XS1_PORT_1A"
    rx_port = None
    checker = UARTTxChecker(rx_port, tx_port, parity, baud, length_of_test, stop, bpb)
    
    tester = px.testers.PytestComparisonTester(f'{cwd}/expected/test_tx_uart_{bpb}b.expect',
                                            regexp = False,
                                            ordered = True,
                                            ignore = ["TEST CONFIG:.*"])

    simargs = ["--trace-to", "trace.txt", "--vcd-tracing", "-tile tile[0] -ports -ports-detailed -cores -instructions -o trace.vcd"] #This is just for local debug so we can capture the run, pass as kwarg to run_with_pyxsim
    px.run_with_pyxsim(binary, simthreads = [checker])
    capture = capfd.readouterr().out[:-1] #Tester appends an extra line feed which we don't need
    tester.run(capture)