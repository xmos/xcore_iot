#!/usr/bin/env python
import xmostest
from spi_slave_checker import SPISlaveChecker
import os


def do_slave_rx_tx(combined, burnt_threads, miso_enable, mode, transfer_size, testlevel):

    resources = xmostest.request_resource("xsim")

    binary = "spi_slave_rx_tx/bin/{com}{burnt}{miso}{m}{t}/spi_slave_rx_tx_{com}{burnt}{miso}{m}{t}.xe".format(com=combined,burnt=burnt_threads,miso=miso_enable,m=mode,t=transfer_size)

    checker = SPISlaveChecker("tile[0]:XS1_PORT_1C",
                               "tile[0]:XS1_PORT_1D",
                               "tile[0]:XS1_PORT_1A",
                               "tile[0]:XS1_PORT_1B",
                               "tile[0]:XS1_PORT_1E",
                               "tile[0]:XS1_PORT_16B",
                               "tile[0]:XS1_PORT_1F")

    tester = xmostest.ComparisonTester(open('slave.expect'),
                                     'lib_spi',
                                     'spi_slave_sim_tests',
                                     'rx_tx_slave_{com}{burnt}{miso}{m}{t}.xe'.format(com=combined,burnt=burnt_threads,miso=miso_enable,m=mode,t=transfer_size),
                                     {'combined': combined, 'burnt_threads': burnt_threads, 'miso_enable': miso_enable, 'mode': mode, 'transfer_size': transfer_size},
                                     regexp=True)

    tester.set_min_testlevel(testlevel)

    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads = [checker],
                              #simargs=['--vcd-tracing', '-o ./spi_slave_rx_tx/trace.vcd -tile tile[0] -pads -functions -clock-blocks -ports-detailed -instructions'],
                              simargs=[],
                              suppress_multidrive_messages = False,
                              tester = tester)

def runtest():
  for transfer_size in [8, 32]:
    for miso_enable in [0, 1]:
      do_slave_rx_tx(1, 2+1, miso_enable, 3, transfer_size, "smoke")

  for combined in [0,1]:
    for mode in range(0, 4):
      for burnt_threads in [2, 6]:
        for transfer_size in [8, 32]:
          for miso_enable in [0, 1]:
            do_slave_rx_tx(combined, burnt_threads+combined, miso_enable, mode, transfer_size, "nightly")
