# Copyright 2014-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import xmostest
from i2c_master_checker import I2CMasterChecker
import os

def do_test(stop, port_setup):
    resources = xmostest.request_resource("xsim")
    speed = 400

    binary = 'i2c_master_test/bin/i2c_master_test_rx_tx_%(speed)d_%(stop)s_%(port_setup)d/i2c_master_test_rx_tx_%(speed)d_%(stop)s_%(port_setup)d.xe' % {
        'speed'      : speed,
        'stop'       : stop,
        'port_setup' : port_setup
    }

    port_map = [["tile[0]:XS1_PORT_1A", "tile[0]:XS1_PORT_1B"],     # Test 1b port SCL 1b port SDA
                ["tile[0]:XS1_PORT_8A.1", "tile[0]:XS1_PORT_8A.3"], # Test 8b port shared by SCL and SDA
                ["tile[0]:XS1_PORT_8A", "tile[0]:XS1_PORT_8B"],     # Test 8b port SCL 8b port SDA
                ["tile[0]:XS1_PORT_1M", "tile[0]:XS1_PORT_8D.1"],   # Test 1b port SCL with overlapping 8b port SDA
                ["tile[0]:XS1_PORT_8D.1", "tile[0]:XS1_PORT_1M"]]   # Test 8b port SCL with overlapping 1b port SDA

    checker = I2CMasterChecker(port_map[port_setup][0],
                               port_map[port_setup][1],
                               tx_data = [0x99, 0x3A, 0xff],
                               expected_speed=170,
                               clock_stretch=5000,
                               ack_sequence=[True, True, False,
                                             True,
                                             True,
                                             True, True, True, False,
                                             True, False])

    tester = xmostest.ComparisonTester(open('expected/master_test_%s.expect' % stop),
                                     'lib_i2c', 'i2c_master_sim_tests',
                                      'clock_stretch',
                                      {'speed' : speed, 'stop' : stop, 'port_setup' : port_setup},
                                     regexp=True)

    # vcd_args = '-o test.vcd'
    # vcd_args += ( ' -tile tile[0] -ports -ports-detailed -instructions'
    #   ' -functions -cycles -clock-blocks -pads' )

    sim_args = ['--weak-external-drive']
    # sim_args += [ '--vcd-tracing', vcd_args ]

    xmostest.run_on_simulator(resources['xsim'], binary,
                              simthreads = [checker],
                              simargs=sim_args,
                              suppress_multidrive_messages = True,
                              tester = tester)

def runtest():
  for stop in ['stop', 'no_stop']:
      # for speed in [400, 100, 10]:
      for port_setup in [0, 1, 2, 3, 4]:
            do_test(stop, port_setup)
