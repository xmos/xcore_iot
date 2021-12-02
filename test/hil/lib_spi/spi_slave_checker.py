# Copyright 2015-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
from functools import partial

# We need to disable output buffering for this test to work on MacOS; this has
# no effect on Linux systems. Let's redefine print once to avoid putting the 
# same argument everywhere.
print = partial(print, flush=True)


class SPISlaveChecker(px.SimThread):
    """"
    This simulator thread will act as SPI slave and check any transactions
    caused by the master.
    """
    def __init__(self, 
                 sck_port: str, 
                 mosi_port: str, 
                 miso_port: str, 
                 ss_port: str, 
                 setup_strobe_port: str, 
                 setup_data_port: str, 
                 setup_resp_port: str) -> None:
        self._miso_port = miso_port
        self._mosi_port = mosi_port
        self._sck_port = sck_port
        self._ss_port = ss_port
        self._setup_strobe_port = setup_strobe_port
        self._setup_data_port = setup_data_port
        self._setup_resp_port = setup_resp_port

    def get_setup_data(self, 
                       xsi: px.pyxsim.Xsi, 
                       setup_strobe_port: str, 
                       setup_data_port: str) -> int:
        self.wait_for_port_pins_change([setup_strobe_port])
        self.wait_for_port_pins_change([setup_strobe_port])
        return xsi.sample_port_pins(setup_data_port)

    def run(self):
        xsi: px.pyxsim.Xsi = self.xsi
        xsi.drive_port_pins(self._ss_port,1)

        print("SPI Slave checker started")
        while True:
            #first do the setup rx
            strobe_val = xsi.sample_port_pins(self._setup_strobe_port)
            if strobe_val == 1:
                xsi.drive_port_pins(self._sck_port, expected_cpol)
                xsi.drive_port_pins(self._ss_port, 1)
                self.wait_for_port_pins_change([self._setup_strobe_port])

            expected_cpol = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_cpha = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_miso_enabled = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_num_bits = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            kbps = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            initial_clock_delay = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port) * 1000
            print(f"Got Settings:cpol {expected_cpol} cpha {expected_cpha} miso {expected_miso_enabled} num_bits {expected_num_bits} kbps {kbps} init delay {initial_clock_delay} ")

            # drive initial values while slave starts up for the first time
            xsi.drive_port_pins(self._sck_port, expected_cpol)
            xsi.drive_port_pins(self._ss_port, 1)
            self.wait_until(xsi.get_time() + 10000000)

            xsi.drive_port_pins(self._sck_port, expected_cpol)
            xsi.drive_port_pins(self._ss_port, 0)
            self.wait_until(xsi.get_time() + initial_clock_delay)

            tx_data = [0xaa, 0xf7, 0xfb, 0xef, 0xdf, 0xbf, 0xfd, 0x7f, 0x01, 0x08, 0x04, 0x10, 0x20, 0x04, 0x02, 0x80]
            rx_data = [0xaa, 0x02, 0x04, 0x08, 0x10, 0x20, 0x04, 0x80, 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f]
            rx_byte = 0
            tx_byte = tx_data[0]

            clock_val = (expected_cpol^expected_cpha)&1
            bit_count = 0
            total_bit_count = 0
            byte_count = 0

            half_clock = 1000000000/(2*kbps)
            error = 0

            while total_bit_count < expected_num_bits:
                # print "Drive bit %d" % total_bit_count
                #clock edge and drive data out
                xsi.drive_port_pins(self._sck_port, clock_val)
                xsi.drive_port_pins(self._mosi_port,(tx_byte>>7)&1)
                tx_byte = tx_byte<<1
                self.wait_until(xsi.get_time() + half_clock)

                #clockedge and read data in
                xsi.drive_port_pins(self._sck_port, 1-clock_val)
                val = xsi.sample_port_pins(self._miso_port)
                self.wait_until(xsi.get_time() + half_clock)
                rx_byte = (rx_byte<<1) + val
                bit_count = bit_count + 1
                total_bit_count = total_bit_count + 1
                if bit_count == 8:
                    bit_count = 0
                    if expected_miso_enabled:
                        if rx_byte != rx_data[byte_count]:
                            error = 1
                            print(f"frx got:{rx_byte:02x} expected:{rx_data[byte_count]:02x}  {byte_count}")
                    rx_byte = 0
                    byte_count = byte_count + 1
                    if byte_count*8 < expected_num_bits:
                        tx_byte = tx_data[byte_count]

            #check the final few rx'd bits
            if bit_count and expected_miso_enabled:
                if rx_byte != rx_data[byte_count]>>(8-bit_count):
                    error = 1
                    print(f"sub bit rx got:{rx_byte:02x} expected:{rx_data[byte_count]>>(8-bit_count):02x}  {byte_count}")
            self.wait_until(xsi.get_time() + half_clock)

            xsi.drive_port_pins(self._sck_port, expected_cpol)
            xsi.drive_port_pins(self._ss_port, 1)

            self.wait_for_port_pins_change([self._setup_strobe_port])
            xsi.drive_port_pins(self._sck_port, expected_cpol)
            xsi.drive_port_pins(self._ss_port, 1)
            xsi.drive_port_pins(self._setup_resp_port, error)
            self.wait_for_port_pins_change([self._setup_strobe_port])
