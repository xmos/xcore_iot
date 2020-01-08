#!/usr/bin/env python
import xmostest
from spi_slave_checker import SPISlaveChecker
import os


def do_slave_benchmark(combined, burnt_threads, miso_enable, mode, transfer_size):
    resources = xmostest.request_resource("xsim")

    binary = "spi_slave_benchmark/bin/{com}{burnt}{miso}{m}{t}/spi_slave_benchmark_{com}{burnt}{miso}{m}{t}.xe".format(com=combined,burnt=burnt_threads,miso=miso_enable,m=mode,t=transfer_size)

    checker = SPISlaveChecker("tile[0]:XS1_PORT_1C",
                               "tile[0]:XS1_PORT_1D",
                               "tile[0]:XS1_PORT_1A",
                               "tile[0]:XS1_PORT_1B",
                               "tile[0]:XS1_PORT_1E",
                               "tile[0]:XS1_PORT_16B",
                               "tile[0]:XS1_PORT_1F")

    tester = xmostest.ComparisonTester(open('slave_benchmark.expect'),
                                     'lib_spi',
                                     'spi_slave_sim_tests',
                                     'spi_slave_benchmark_{com}{burnt}{miso}{m}{t}.xe'.format(com=combined,burnt=burnt_threads,miso=miso_enable,m=mode,t=transfer_size),
                                     regexp=True)

    tester.set_min_testlevel('nightly')
    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads = [checker],
                              #simargs=['--vcd-tracing', '-o ./spi_slave_benchmark/trace.vcd -tile tile[0] -pads -functions -clock-blocks -ports-detailed -instructions'],
                              simargs=[],
                              suppress_multidrive_messages = False,
                              tester = tester)

def runtest():
  for combined in [0,1]:
    for mode in range(0, 4):
      for burnt_threads in [2, 6]:
        for transfer_size in [8, 32]:
          for miso_enable in [0, 1]:
            do_slave_benchmark(combined, burnt_threads+combined, miso_enable, mode, transfer_size)
