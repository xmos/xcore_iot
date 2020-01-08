#!/usr/bin/env python
import xmostest
from spi_master_checker import SPIMasterChecker

if __name__ == "__main__":
    xmostest.init()

    xmostest.register_group("lib_spi",
                            "spi_master_sim_tests",
                            "spi master simulator tests",
    """
Tests are performed by running the spi library connected to a
simulator model (written as a python plugin to xsim). The simulator
model checks that the signals comply to the spi specification and checks the
protocol speed of the transactions. Tests are run to test the following
features:

    * Transmission of packets
    * Reception of packets

The tests are run with transactions of varying number of bytes and with rx and
tx transactions interleaved. The tests are run at speeds of 10, 100 and 400
Kbps.
""")

    xmostest.register_group("lib_spi",
                            "spi_slave_sim_tests",
                            "spi Slave simulator tests",
    """
Tests are performed by running the spi library connected to a
simulator model (written as a python plugin to xsim). The simulator
model checks that the signals comply to the spi specification and checks the
protocol speed of the transactions. Tests are run to test the following
features:

    * Transmission of packets
    * Reception of packets

The tests are run with transactions of varying number of bytes and with rx and
tx transactions interleaved. The tests are run at speeds of 10, 100 and 400
Kbps.
""")

    xmostest.runtests()

    xmostest.finish()
