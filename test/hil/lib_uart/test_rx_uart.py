import xmostest

from uart_rx_checker import UARTRxChecker, Parity


def do_test(baud):
    myenv = {'baud': baud}
    path = "app_uart_test_rx"
    resources = xmostest.request_resource("xsim")

    checker = UARTRxChecker("tile[0]:XS1_PORT_1A", "tile[0]:XS1_PORT_1B", Parity['UART_PARITY_NONE'], baud, 1, 8)
    tester = xmostest.ComparisonTester(open('test_rx_uart.expect'),
                                       "lib_uart", "sim_regression", "rx", myenv,
                                       regexp=True)

    if baud != 115200:
        tester.set_min_testlevel('nightly')

    xmostest.run_on_simulator(resources['xsim'],
                              'app_uart_test_rx/bin/smoke/app_uart_test_rx_smoke.xe',
                              simthreads=[checker],
                              xscope_io=True,
                              tester=tester,
                              simargs=["--vcd-tracing", "-tile tile[0] -ports -o trace.vcd"],
                              clean_before_build=True,
                              build_env=myenv)


def runtest():
    for baud in [14400, 28800, 57600, 115200]:
        do_test(baud)
