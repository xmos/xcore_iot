# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
from typing import Sequence
from functools import partial

# We need to disable output buffering for this test to work on MacOS; this has
# no effect on Linux systems. Let's redefine print once to avoid putting the 
# same argument everywhere.
print = partial(print, flush=True)


class UARTTxChecker(px.SimThread):
    """
    This simulator thread will act as a UART device, and will check sent and
    transations caused by the device, by looking at the tx pins.
    """

    def __init__(self, rx_port, tx_port, parity, baud, length, stop_bits, bpb):
        """
        Create a UARTTxChecker instance.

        :param rx_port:    Receive port of the UART device under test.
        :param tx_port:    Transmit port of the UART device under test.
        :param parity:     Parity of the UART connection.
        :param baud:       BAUD rate of the UART connection.
        :param length:     Length of transmission to check.
        :param stop_bits:  Number of stop_bits for each UART byte.
        :param bpb:        Number of data bits per "byte" of UART data.
        """
        self._rx_port = rx_port
        self._tx_port = tx_port
        self._parity = parity
        self._baud = baud
        self._length = length
        self._stop_bits = stop_bits
        self._bits_per_byte = bpb
        # Hex value of stop bits, as MSB 1st char, e.g. 0b11 : 0xC0

    def get_port_val(self, xsi, port):
        """
        Sample the state of a port

        :rtype:            int
        :param xsi:        XMOS Simulator Instance.
        :param port:       Port to sample.
        """
        is_driving = xsi.is_port_driving(port)
        if not is_driving:
            return 1
        else:
            return xsi.sample_port_pins(port)

    def get_bit_time(self):
        """
        Returns the expected time between bits for the currently set BAUD rate.

        Returns float value in nanoseconds.
        :rtype:            float
        """
        # Return float value in ps
        return (1.0/self._baud) * 1e12

    def wait_baud_time(self, xsi):
        """
        Wait for 1 bit time, as determined by the baud rate.
        """
        self.wait_until(xsi.get_time() + self.get_bit_time())
        return True

    def wait_half_baud_time(self, xsi):
        """
        Wait for half a bit time, as determined by the baud rate.
        """
        self.wait_until(xsi.get_time() + (self.get_bit_time() / 2))

    def read_packet(self, xsi, parity, length=4):
        """
        Read a given number of bytes of UART traffic sent by the device.

        Returns a list of bytes sent by the device.

        :rtype:            list
        :param xsi:        XMOS Simulator Instance.
        :param parity:     The UART partiy setting. See Parity.
        :param length:     The number of bytes to read. Defaults to 4.
        """
        packet = []
        start_time = 0
        got_start_bit = False

        initial_port_val = self.get_port_val(xsi, self._tx_port)
        print("tx starts high: %s" % ("True" if initial_port_val else "False"))

        for x in range(length):
            packet.append(chr(self.read_byte(xsi, parity)))
        return packet

    def read_byte(self, xsi, parity):
        """
        Read 1 byte of UART traffic sent by the device

        Returns an int, representing a byte read from the uart. Should be in the range 0 <= x < 2^bits_per_byte

        :rtype:            int
        :param xsi:        XMOS Simulator Instance.
        :param parity:     The UART partiy setting. See Parity.
        """
        byte = 0
        val = 0

        # Recv start bit
        initial_port_val = self.get_port_val(xsi, self._tx_port)

        if initial_port_val == 1:
            self.wait_for_port_pins_change([self._tx_port])
        #else go for it as assume tx has just fallen with no interframe gap
        # print("Byte start time: ", xsi.get_time())

        # The tx line should go low for 1 bit time
        if self.get_val_timeout(xsi, self._tx_port) == 0:
            print("Start bit recv'd")
        else:
            print("Start bit issue")
            return False

        # recv the byte
        crc_sum = 0
        for j in range(self._bits_per_byte):
            val = self.get_val_timeout(xsi, self._tx_port)
            byte += (val << j)
            crc_sum += val

        print(f"Sampled {self._bits_per_byte} data bits")

        # Check the parity if needs be
        self.check_parity(xsi, crc_sum, parity)

        # Get the stop bit
        self.check_stopbit(xsi)

        # Print a new line to split bytes in output
        print()

        return byte

    def check_parity(self, xsi, crc_sum, parity):
        """
        Read the parity bit and check it against a crc sum. Print correctness.

        :param xsi:        XMOS Simulator Instance.
        :param crc_sum:    The checksum to test parity against.
        :param parity:     The UART partiy setting. See Parity.
        """
        if parity > 0:
            parity_val = 0 if parity == 1 else 1
            read = self.get_val_timeout(xsi, self._tx_port)
            if read == (crc_sum + parity_val) % 2:
                print("Parity bit correct")
            else:
                print("Parity bit incorrect. Got %d, expected %d" % (read, (crc_sum + parity_val) % 2))
        else:
            print("Parity bit correct")

    def check_stopbit(self, xsi):
        """
        Read the stop bit(s) of a UART transmission and print correctness.

        :param xsi:        XMOS Simulator Instance.
        """
        stop_bits_correct = True
        for i in range(self._stop_bits):
            # The stop bits should stay high for this time
            if self.get_val_timeout(xsi, self._tx_port) == 0:
                stop_bits_correct = False
        print("Stop bit correct: %s" % ("True" if stop_bits_correct else "False"))

    def get_val_timeout(self, xsi, port):
        """
        Get a value from a given port of the device, with a timeout determined
        by the BAUD rate.

        Returns whether the pin is high (True) or low (False)

        :rtype:            bool
        :param xsi:        XMOS Simulator Instance.
        :param port:       The port to sample.
        """
        # This intentionally has a 0.3% slop. It is per-byte and gives some
        # wiggle-room if the clock doesn't divide into ns nicely.
        timeout = self.get_bit_time() * 0.5
        short_timeout = self.get_bit_time() * 0.2485

        # Allow for "rise" time
        self.wait_until(xsi.get_time() + short_timeout)

        # Get val
        K = self.wait_time_or_pin_change(xsi, timeout, port)

        # Allow for "fall" time
        self.wait_until(xsi.get_time() + short_timeout)
        return K

    def wait_time_or_pin_change(self, xsi, timeout, port):
        """
        Waits for a given timeout, or until a port changes state. Which ever
        occurs 1st. Prints an error if the former causes the function to break.

        Returns whether the pin is high (True) or low (False)

        :rtype:            bool
        :param xsi:        XMOS Simulator Instance.
        :param timeout:    Time to wait.
        :param port:       Port to sample.
        """
        start_time = xsi.get_time()
        start_val = self.get_port_val(xsi, port)
        transitioned_during_wait = False

        def _continue(_timeout, _start_time, _start_val):
            if xsi.get_time() >= _start_time + _timeout:
                return True
            if self.get_port_val(xsi, port) != _start_val:
                transitioned_during_wait = True
                return True
            return False
        wait_fun = (lambda x: _continue(timeout, start_time, start_val))
        self.wait(wait_fun)

        # Start value should *not* have changed during timeout
        if transitioned_during_wait:
            print("FAIL :: Unexpected Transition.")

        return start_val

    def run(self):
        # Wait for the xcore to bring the uart tx port up
        self.wait((lambda x: self.xsi.is_port_driving(self._tx_port)))
        self.wait((lambda x: self.get_port_val(self.xsi, self._tx_port) == 1))

        K = self.read_packet(self.xsi, self._parity, self._length)

        # Print each member of K as a hex byte
        # inline lambda function mapped over a list? awh yiss.
        print(", ".join(map((lambda x: "0x%02x" % ord(x)), K)))

