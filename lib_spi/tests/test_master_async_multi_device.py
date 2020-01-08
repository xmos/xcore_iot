#!/usr/bin/env python
import xmostest
from spi_master_checker import SPIMasterChecker
import os


def do_multi_device_async(burnt_threads, mosi_enable):
    resources = xmostest.request_resource("xsim")

    binary = "spi_master_async_multi_device/bin/{burnt}{mosi}/spi_master_async_multi_device_{burnt}{mosi}.xe".format(burnt=burnt_threads,mosi=mosi_enable)

    checker = SPIMasterChecker("tile[0]:XS1_PORT_1C",
                               "tile[0]:XS1_PORT_1D",
                               "tile[0]:XS1_PORT_1A",
                               ["tile[0]:XS1_PORT_1B", "tile[0]:XS1_PORT_1G"],
                               "tile[0]:XS1_PORT_1E",
                               "tile[0]:XS1_PORT_16B")

    tester = xmostest.ComparisonTester(open('master.expect'),
                                     'lib_spi',
                                     'spi_master_sim_tests',
                                     'spi_master_async_multi_device_{burnt}{mosi}'.format(burnt=burnt_threads, mosi=mosi_enable),
                                     regexp=True)

    tester.set_min_testlevel('nightly')
    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads = [checker],
                              #simargs=['--vcd-tracing', '-o ./spi_master_async_multi_device/trace.vcd -tile tile[0] -pads -functions'],
                              simargs=[],
                              suppress_multidrive_messages = False,
                              tester = tester)

def runtest():
    for burnt_threads in [2, 6]:
      for mosi_enabled in [0, 1]:
          do_multi_device_async(burnt_threads, mosi_enabled)
