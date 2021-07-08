# Copyright 2016-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import xmostest
from i2s_master_checker import I2SMasterChecker
from i2s_master_checker import Clock
import os

def do_test(sample_rate, num_channels, receive_increment, send_increment, testlevel):

    resources = xmostest.request_resource("xsim")

    binary = 'backpressure_test/bin/backpressure_test_{sr}_{nc}_{ri}_{si}/backpressure_test_{sr}_{nc}_{ri}_{si}.xe'.format(
      sr=sample_rate, nc=num_channels, ri=receive_increment, si=send_increment)

    tester = xmostest.ComparisonTester(
      open('expected/backpressure_test.expect'),
       'lib_i2s', 'i2s_backpressure_tests', 'backpressure_%s'%testlevel,
       {'sample_rate':sample_rate,
        'num_channels':num_channels,
        'receive_increment':receive_increment,
        'send_increment':send_increment})

    tester.set_min_testlevel(testlevel)

    xmostest.run_on_simulator(resources['xsim'], binary,
                              simargs=['--xscope', '-offline xscope.xmt'],
                              # simargs=['--xscope', '-offline xscope.xmt', '--trace-to', 'sim.log', '--vcd-tracing', '-o ./backpressure_test/trace.vcd -tile tile[0] -ports-detailed -functions'],
                              loopback=[{'from': 'tile[0]:XS1_PORT_1G',
                                         'to': 'tile[0]:XS1_PORT_1A'}],
                              suppress_multidrive_messages=True,
                              tester=tester)

def runtest():
  for sample_rate in [768000, 384000, 192000]:
    for num_channels in [1, 2, 3, 4]:
      do_test(sample_rate, num_channels, 5,  5, "smoke" if (num_channels == 4) else "nightly")
      do_test(sample_rate, num_channels, 0, 10, "smoke" if (num_channels == 4) else "nightly")
      do_test(sample_rate, num_channels, 10, 0, "smoke" if (num_channels == 4) else "nightly")
