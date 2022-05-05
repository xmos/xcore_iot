#!/usr/bin/env python
# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

from uart_rx_checker import UARTRxChecker
from pathlib import Path
import Pyxsim as px
import pytest
import sys

speed_args = {"1843200 baud": 1843200,
              "921600 baud": 921600,
              "576000 baud": 576000,
              "115200 baud": 115200,
              "38400 baud": 38400,
              "19200 baud": 19200,
              "9600 baud": 9600,
              "1200 baud": 1200}

data_bit_args = {   "eight": 8,            
                    "seven": 7,
                    "six": 6,
                    "five": 5}

parity_args = { "NONE": 0,
                "EVEN": 1,
                "ODD": 2}                

stop_bit_args = {   "one": 1,
                    "two": 2}

speed_args = {"115200 baud": 115200}
data_bit_args = {"eight": 8}
parity_args = { "NONE": 0}                
stop_bit_args = {"one": 1}

@pytest.mark.parametrize("baud", speed_args.values(), ids=speed_args.keys())
@pytest.mark.parametrize("bpb", data_bit_args.values(), ids=data_bit_args.keys())
@pytest.mark.parametrize("parity", parity_args.values(), ids=parity_args.keys())
@pytest.mark.parametrize("stop", stop_bit_args.values(), ids=stop_bit_args.keys())
def test_uart_rx(request, capfd, baud, bpb, parity, stop):
    myenv = {'baud': baud}
    cwd = Path(request.fspath).parent
    binary = f'{cwd}/uart_test_rx/bin/test_hil_uart_rx_test.xe'

    tx_port = "tile[0]:XS1_PORT_1A" #Used for synch to start checker
    rx_port = "tile[0]:XS1_PORT_1B"
    checker = UARTRxChecker(rx_port, tx_port, parity, baud, stop, bpb, data=[0x7f, 0x00, 0x2f, 0xff])
    
    tester = px.testers.PytestComparisonTester(f'{cwd}/expected/test_rx_uart.expect',
                                            regexp = False,
                                            ordered = True,
                                            ignore = ["TEST CONFIG:.*"])

    simargs = ["--trace-to", "trace.txt", "--vcd-tracing", "-tile tile[0] -ports -ports-detailed -cores -instructions -o trace.vcd"] #This is just for local debug so we can capture the run, pass as kwarg to run_with_pyxsim
    px.run_with_pyxsim(binary, simthreads = [checker], simargs=simargs)
    capture = capfd.readouterr().out[:-1] #Tester appends an extra line feed which we don't need

    tester.run(capture)