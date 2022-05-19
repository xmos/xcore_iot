# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
from typing import Sequence
from functools import partial

# We need to disable output buffering for this test to work on MacOS; this has
# no effect on Linux systems. Let's redefine print once to avoid putting the 
# same argument everywhere.
print = partial(print, flush=True)

Parity = dict(
    UART_PARITY_EVEN=0,
    UART_PARITY_ODD=1,
    UART_PARITY_NONE=2,
    UART_PARITY_BAD=3
)


class DriveHigh(px.SimThread):
    def __init__(self, p):
        self._p = p

    def run(self):
        xsi = self.xsi

        xsi.drive_port_pins(self._p, 1);


class UARTRxChecker(px.SimThread):
    def __init__(self, rx_port, tx_port, parity, baud, stop_bits, bpb, data=[0x7f, 0x00, 0x2f, 0xff],
                 intermittent=False):
        """
        Create a UARTRxChecker instance.

        :param rx_port:    Receive port of the UART device under test.
        :param tx_port:    Transmit port of the UART device under test.
        :param parity:     Parity of the UART connection.
        :param baud:       BAUD rate of the UART connection.
        :param stop_bits:  Number of stop_bits for each UART byte.
        :param bpb:        Number of data bits per "byte" of UART data.
        :param data:       A list of bytes to send (default: [0x7f, 0x00, 0x2f, 0xff])
        :param intermittent: Add a random delay between sent bytes.
        """
        self._rx_port = rx_port
        self._tx_port = tx_port
        self._parity = parity
        self._baud = baud
        self._stop_bits = stop_bits
        self._bits_per_byte = bpb
        self._data = data
        self._intermittent = intermittent
        # Hex value of stop bits, as MSB 1st char, e.g. 0b11 : 0xC0

    def send_byte(self, xsi, byte):
        """
        Send a byte to the rx_port

        :param xsi:        XMOS Simulator Instance.
        :param byte:       Byte to send
        """
        # Send start bit
        self.send_start(xsi)

        # Send data
        self.send_data(xsi, byte)

        # Send parity
        self.send_parity(xsi, byte)

        # Send stop bit(s)
        self.send_stop(xsi)


    def send_start(self, xsi):
        """
        Send a start bit.

        :param xsi:        XMOS Simulator Instance.
        """
        xsi.drive_port_pins(self._rx_port, 0)
        self.wait_baud_time(xsi)

    def send_data(self, xsi, byte):
        """
        Write the data bits to the rx_port

        :param xsi:        XMOS Simulator Instance.
        :param byte:       Data to send.
        """
        # print "0x%02x:" % byte
        for x in range(self._bits_per_byte):
            # print "  Sending bit %d of 0x%02x (%d)" % (x, byte, (byte >> x) & 0x01)
            xsi.drive_port_pins(self._rx_port, (byte & (0x01 << x)) >= 1)
            # print "  (x): %d" % ((byte & (0x01 << x))>=1)
            self.wait_baud_time(xsi)

    def send_parity(self, xsi, byte):
        """
        Send the parity bit to the rx_port

        :param xsi:        XMOS Simulator Instance.
        :param byte:       Data to send parity of.
        """
        parity = (self._parity - 1) % 3 #parity enum in lib_uart (old XC) different from SDK
        if parity < 2:
            crc_sum = 0
            for x in range(self._bits_per_byte):
                crc_sum += ((byte & (0x01 << x)) >= 1)
            crc_sum += parity
            # print "Parity for 0x%02x: %d" % (byte, crc_sum%2)
            xsi.drive_port_pins(self._rx_port, crc_sum % 2)
            self.wait_baud_time(xsi)
        elif parity == Parity['UART_PARITY_BAD']:
            # print "Sending bad parity bit"
            self.send_bad_parity(xsi)

    def send_stop(self, xsi):
        """
        Send the stop bit(s) to the rx_port

        :param xsi:        XMOS Simulator Instance.
        """
        for x in range(self._stop_bits):
            xsi.drive_port_pins(self._rx_port, 1)
            self.wait_baud_time(xsi)

    def send_bad_parity(self, xsi):
        """
        Send a parity bit of 1 to simulate an incorrect parity state.

        :param xsi:        XMOS Simulator Instance.
        """
        # Always send a parity bit of 1
        xsi.drive_port_pins(self._rx_port, 0)
        self.wait_baud_time(xsi)

    def get_bit_time(self):
        """
        Returns the expected time between bits for the currently set BAUD rate.

        Returns float value in nanoseconds.
        """
        # Return float value in ps
        return (1.0 / self._baud) * 1e12

    def wait_baud_time(self, xsi):
        """
        Wait for 1 bit time, as determined by the baud rate.
        """
        self.wait_until(xsi.get_time() + self.get_bit_time())

    def wait_half_baud_time(self, xsi):
        """
        Wait for half a bit time, as determined by the baud rate.
        """
        self.wait_until(xsi.get_time() + (self.get_bit_time() / 2))

    def run(self):
        xsi = self.xsi
        # Drive the uart line high.
        xsi.drive_port_pins(self._rx_port, 1)

        # Wait for the device to bring up it's tx port, indicating it is ready
        self.wait((lambda _x: self.xsi.is_port_driving(self._tx_port)))

        # If we're doing an intermittent send, add a delay between each byte
        # sent. Delay is in ns. 20,000ns = 20ms, 100,000ns = 100ms. Delays could
        # be more variable, but it hurts test time substantially.
        if self._intermittent:
            for x in self._data:
                k = randint(20000, 100000)
                self.wait_until(xsi.get_time() + k)
                self.send_byte(xsi, x)
        else:
            for x in self._data:
                self.send_byte(xsi, x)
