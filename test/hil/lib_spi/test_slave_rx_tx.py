#!/usr/bin/env python
# Copyright 2015-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import xmostest
from spi_slave_checker import SPISlaveChecker
import os


def do_slave_rx_tx(full_load, miso_enable, mosi_enable, mode, in_place):

    resources = xmostest.request_resource("xsim")

    binary = "spi_slave_rx_tx/bin/spi_slave_rx_tx_{load}_{miso}_{mosi}_{m}_{in_place}/spi_slave_rx_tx_{load}_{miso}_{mosi}_{m}_{in_place}.xe".format(load=full_load,miso=miso_enable,mosi=mosi_enable,m=mode,in_place=in_place)

    checker = SPISlaveChecker("tile[0]:XS1_PORT_1C",
                              "tile[0]:XS1_PORT_1D",
                              "tile[0]:XS1_PORT_1A",
                              "tile[0]:XS1_PORT_1B",
                              "tile[0]:XS1_PORT_1E",
                              "tile[0]:XS1_PORT_16B",
                              "tile[0]:XS1_PORT_1F")

    tester = xmostest.ComparisonTester(open('expected/slave.expect'),
                                     'lib_spi',
                                     'spi_slave_sim_tests',
                                     'rx_tx_slave_{load}_{miso}_{mosi}_{m}_{in_place}.xe'.format(load=full_load,miso=miso_enable,mosi=mosi_enable,m=mode,in_place=in_place),
                                     {'full_load': full_load, 'miso_enable': miso_enable, 'mosi_enable': mosi_enable, 'mode': mode, 'in_place': in_place},
                                     regexp=True)

    if full_load == 0:
        tester.set_min_testlevel('nightly')

    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads = [checker],
                              # simargs=['--vcd-tracing', '-o ./spi_slave_rx_tx/trace{load}_{miso}_{mosi}_{m}_{in_place}.vcd -tile tile[0] -pads -functions -clock-blocks -ports-detailed -instructions'.format(load=full_load,miso=miso_enable,mosi=mosi_enable,m=mode,in_place=in_place)],
                              simargs=[],
                              suppress_multidrive_messages = False,
                              tester = tester)

def runtest():
    for full_load in [0, 1]:
        for miso_enable in [0, 1]:
            for mosi_enable in [0, 1]:
                for mode in range(0, 4):
                    for in_place in [0, 1]:
                        if not (mosi_enable == 0 and miso_enable == 0):
                            do_slave_rx_tx(full_load, miso_enable, mosi_enable, mode, in_place)
