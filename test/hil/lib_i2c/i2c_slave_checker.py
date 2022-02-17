# Copyright 2014-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
from typing import Sequence, Union, Tuple, Literal
from Pyxsim import SimThread, pyxsim
from functools import partial

# We need to disable output buffering for this test to work on MacOS; this has
# no effect on Linux systems. Let's redefine print once to avoid putting the
# same argument everywhere.
print = partial(print, flush=True)


class I2CSlaveChecker(SimThread):
    """
    This simulator thread will act as I2C master, create
    bus transactions and test the response of the slave
    """

    def __init__(
        self,
        scl_port: str,
        sda_port: str,
        speed: int,
        tsequence: Sequence[Tuple[Literal["r", "w"], Union[Sequence[int], int]]],
    ) -> None:
        self._scl_port = scl_port
        self._sda_port = sda_port
        self._tsequence = tsequence
        self._speed = speed
        self._bit_time = 1000000000 / speed
        print(f"Checking I2C: SCL={self._scl_port}, SDA={self._sda_port}")

    def get_port_val(self, xsi: pyxsim.Xsi, port: str) -> int:
        """
        Sample port, modelling the pull up
        """
        is_driving = xsi.is_port_driving(port)
        if not is_driving:
            return 1
        else:
            return xsi.sample_port_pins(port)

    def start_bit(self, xsi: pyxsim.Xsi) -> None:
        xsi.drive_port_pins(self._scl_port, 1)
        self.wait_until(xsi.get_time() + self._bit_time / 4)
        xsi.drive_port_pins(self._sda_port, 0)
        self.wait_until(xsi.get_time() + self._bit_time / 2)
        xsi.drive_port_pins(self._scl_port, 0)
        self._fall_time = xsi.get_time()

    def high_pulse(self, xsi: pyxsim.Xsi) -> None:
        self.wait_until(self._fall_time + self._bit_time / 2 + self._bit_time / 32)
        xsi.drive_port_pins(self._scl_port, 1)
        if xsi.is_port_driving(self._scl_port):
            self.wait_for_port_pins_change([self._scl_port])
        new_fall_time = self._fall_time + self._bit_time
        if xsi.get_time() > new_fall_time:
            new_fall_time = xsi.get_time() + self._bit_time / 4
        self.wait_until(new_fall_time)
        xsi.drive_port_pins(self._scl_port, 0)
        self._fall_time = new_fall_time

    def high_pulse_sample(self, xsi: pyxsim.Xsi) -> int:
        self.wait_until(self._fall_time + self._bit_time / 2 + self._bit_time / 32)
        if xsi.is_port_driving(self._scl_port):
            self.wait_for_port_pins_change([self._scl_port])
        xsi.drive_port_pins(self._scl_port, 1)
        self.wait_until(xsi.get_time() + self._bit_time / 4)
        data = self.get_port_val(xsi, self._sda_port)
        new_fall_time = self._fall_time + self._bit_time
        if xsi.get_time() > new_fall_time:
            new_fall_time = xsi.get_time() + self._bit_time / 4
        self.wait_until(new_fall_time)
        xsi.drive_port_pins(self._scl_port, 0)
        self._fall_time = new_fall_time
        return data

    def write(self, xsi: pyxsim.Xsi, byte: int) -> None:
        print(f"Sending data 0x{byte:x}")
        for _ in range(8):
            self.wait_until(self._fall_time + self._bit_time / 8)
            bit = (byte >> 7) & 1
            xsi.drive_port_pins(self._sda_port, bit)
            byte <<= 1
            self.high_pulse(xsi)
        ack = self.high_pulse_sample(xsi)
        if ack == 1:
            print("Master received NACK")
        else:
            print("Master received ACK")

    def stop_bit(self, xsi: pyxsim.Xsi) -> None:
        print("Sending stop bit")
        self.wait_until(self._fall_time + self._bit_time / 4)
        xsi.drive_port_pins(self._sda_port, 0)
        self.wait_until(self._fall_time + self._bit_time / 2 + self._bit_time / 32)
        xsi.drive_port_pins(self._scl_port, 1)
        self.wait_until(self._fall_time + (self._bit_time * 3 / 4))
        xsi.drive_port_pins(self._sda_port, 1)
        self.wait_until(xsi.get_time() + self._bit_time * 2)

    def read(self, xsi: pyxsim.Xsi, ack: int) -> None:
        byte = 0
        for _ in range(8):
            bit = self.high_pulse_sample(xsi)
            byte = (byte << 1) | bit
        print(f"Received byte 0x{byte:x}")
        self.wait_until(self._fall_time + self._bit_time / 8)
        if ack == 0:
            print("Master sending ACK")
        else:
            print("Master sending NACK")
        xsi.drive_port_pins(self._sda_port, ack)
        self.high_pulse(xsi)

    def run(self) -> None:
        xsi = self.xsi
        xsi.drive_port_pins(self._scl_port, 1)
        xsi.drive_port_pins(self._sda_port, 1)
        self.wait_until(xsi.get_time() + 30000000)
        for (typ, addr, d) in self._tsequence:
            if typ == "w":
                self.start_bit(xsi)
                print(f"Starting write transaction to device id 0x{addr:x}")
                self.write(xsi, (addr << 1) | 0)
                for x in d:
                    self.write(xsi, x)
                self.stop_bit(xsi)
            elif typ == "r":
                self.start_bit(xsi)
                print(f"Starting read transaction to device id 0x{addr:x}")
                self.write(xsi, (addr << 1) | 1)
                for x in range(d - 1):
                    self.read(xsi, 0)
                self.read(xsi, 1)
                self.stop_bit(xsi)
