#!/usr/bin/env python
# Copyright 2015-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import xmostest
from spi_master_checker import SPIMasterChecker
import os

def do_multi_device_sync(full_load, miso_enabled, mosi_enable, div, mode):
    resources = xmostest.request_resource("xsim")

    # binary = "spi_master_sync_multi_device/bin/{load}{cb}{miso}{mosi}{speed}/spi_master_sync_multi_device_{load}{cb}{miso}{mosi}{speed}.xe".format(load=full_load,cb=cb_enabled,miso=miso_enabled,mosi=mosi_enable,speed=speed)


    binary = "spi_master_sync_multi_device/bin/spi_master_sync_multi_device_{load}_{miso}_{mosi}_{div}_{mode}/spi_master_sync_multi_device_{load}_{miso}_{mosi}_{div}_{mode}.xe".format(load=full_load,miso=miso_enabled,mosi=mosi_enable,div=div,mode=mode)

    checker = SPIMasterChecker("tile[0]:XS1_PORT_1C",
                               "tile[0]:XS1_PORT_1D",
                               "tile[0]:XS1_PORT_1A",
                               ["tile[0]:XS1_PORT_4A.0", "tile[0]:XS1_PORT_4A.1"],
                               "tile[0]:XS1_PORT_1E",
                               "tile[0]:XS1_PORT_16B")

    tester = xmostest.ComparisonTester(open('expected/master_multi_device.expect'),
                                     'lib_spi',
                                     'spi_master_sim_tests',
                                     'spi_master_sync_multi_device_{load}_{miso}_{mosi}_{div}_{mode}'.format(load=full_load,miso=miso_enabled,mosi=mosi_enable,div=div,mode=mode),
                                     regexp=True)

    if full_load == 0:
        tester.set_min_testlevel('nightly')

    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads = [checker],
                              # simargs=['--vcd-tracing', '-o ./spi_master_sync_multi_device/trace.vcd -tile tile[0] -pads -functions'],
                              simargs=[],
                              suppress_multidrive_messages = False,
                              tester = tester)

def runtest():
    for full_load in [0, 1]:
        for miso_enabled in [0, 1]:
            for mosi_enabled in [0, 1]:
                for div in [4, 8, 80]:
                    for mode in [0, 1, 2, 3]:
                        if not ((miso_enabled == 0) and (mosi_enabled == 0)):
                            do_multi_device_sync(full_load, miso_enabled, mosi_enabled, div, mode)
