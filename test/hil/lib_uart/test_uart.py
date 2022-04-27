import xmostest


class UartTester(xmostest.Tester):
    def __init__(self):
        super(UartTester, self).__init__()
        self.register_test("lib_uart", "sim_regression", "loopback_test", {})

    def run(self, output):
        result = True
        for line in output:
            if line.find("FAIL") != -1:
                result = False
        xmostest.set_test_result("lib_uart", "sim_regression", "loopback_test",
                                 {}, result, output=''.join(output))


def runtest():
    resources = xmostest.request_resource("xsim")
    # xmostest.run_on_simulator(resources['xsim'],
    #                           'app_uart_test/bin/smoke/app_uart_test_smoke.xe',
    #                           xscope_io=True,
    #                           loopback=[{'from': 'tile[0]:XS1_PORT_1A',
    #                                      'to': 'tile[1]:XS1_PORT_1B'}],
    #                           tester=UartTester())
