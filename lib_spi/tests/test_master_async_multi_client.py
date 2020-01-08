#!/usr/bin/env python
import xmostest
from spi_master_checker import SPIMasterChecker
import os


def do_multi_client_async(burnt_threads, mosi_enable, combine):
    resources = xmostest.request_resource("xsim")

    binary = "spi_master_async_multi_client/bin/{burnt}{mosi}{combined}/spi_master_async_multi_client_{burnt}{mosi}{combined}.xe".format(burnt=burnt_threads,mosi=mosi_enable, combined=combine)

    checker = SPIMasterChecker("tile[0]:XS1_PORT_1C",
                               "tile[0]:XS1_PORT_1D",
                               "tile[0]:XS1_PORT_1A",
                               ["tile[0]:XS1_PORT_1B"],
                               "tile[0]:XS1_PORT_1E",
                               "tile[0]:XS1_PORT_16B")

    tester = xmostest.ComparisonTester(open('master.expect'),
                                     'lib_spi',
                                     'spi_master_sim_tests',
                                     'multi_client_async_{burnt}{mosi}{combined}'.format(burnt=burnt_threads, mosi=mosi_enable, combined=combine),
                                     regexp=True)
    tester.set_min_testlevel('nightly')
    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads = [checker],
                              #simargs=['--vcd-tracing', '-o ./spi_master_async_multi_client/trace.vcd -tile tile[0] -pads -functions'],
                              simargs=[],
                              suppress_multidrive_messages = False,
                              tester = tester)

def runtest():
      burnt_threads = 3
      for mosi_enabled in [0, 1]:
        for combine in [0, 1]:
          do_multi_client_async(burnt_threads+combine, mosi_enabled, combine)
