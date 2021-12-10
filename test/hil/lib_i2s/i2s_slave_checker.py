# Copyright 2015-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
from i2s_master_checker import Clock
import Pyxsim as px
from functools import partial

# We need to disable output buffering for this test to work on MacOS; this has
# no effect on Linux systems. Let's redefine print once to avoid putting the 
# same argument everywhere.
print = partial(print, flush=True)


class I2SSlaveChecker(px.SimThread):
    """ "
    This simulator thread will act as I2S master and check any transactions
    caused by the Slave.
    """

    def __init__(
        self,
        bclk: str,
        lrclk: str,
        din,
        dout,
        setup_strobe_port: str,
        setup_data_port: str,
        setup_resp_port: str,
        c: Clock,
        no_start_msg: bool = False,
        invert_bclk: bool = False,
    ):
        self._din = din
        self._dout = dout
        self._bclk = bclk
        self._lrclk = lrclk
        self._setup_strobe_port = setup_strobe_port
        self._setup_data_port = setup_data_port
        self._setup_resp_port = setup_resp_port
        self._clk = c
        self._no_start_msg = no_start_msg
        self._invert_bclk = invert_bclk

    # May be useful to have time the actual return value of SimThread.wait_until
    def wait_until_ret(self, t: int) -> int:
        self.wait_until(t)
        return t

    def get_setup_data(
        self, xsi: px.pyxsim.Xsi, setup_strobe_port: str, setup_data_port: str
    ) -> int:
        self.wait_for_port_pins_change([setup_strobe_port])
        self.wait_for_port_pins_change([setup_strobe_port])
        return xsi.sample_port_pins(setup_data_port)

    def run(self):

        xsi: px.pyxsim.Xsi = self.xsi

        bits_per_word = 32
        num_frames = 4
        bclk0 = 0
        bclk1 = 1
        din_sample_offset = 0
        first_iteration = True

        xsi.drive_port_pins(self._bclk, bclk1)
        if not self._no_start_msg:
            print("I2S Slave Checker Started")
        while True:
            xsi.drive_port_pins(self._setup_resp_port, 0)
            strobe_val = xsi.sample_port_pins(self._setup_strobe_port)
            if strobe_val == 1:
                self.wait_for_port_pins_change([self._setup_strobe_port])

            bclk_frequency_u = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            bclk_frequency_l = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            num_ins = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            num_outs = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            is_i2s_justified = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            xsi.drive_port_pins(self._bclk, bclk1)
            xsi.drive_port_pins(self._lrclk, 1)

            bclk_frequency = (bclk_frequency_u << 16) + bclk_frequency_l
            print(
                f"CONFIG: bclk:{bclk_frequency} in:{num_ins} out:{num_outs} i2s_justified:{is_i2s_justified}"
            )
            clock_half_period = float(1000000000000) / float(2 * bclk_frequency)

            if self._invert_bclk:
                bclk0 = 1
                bclk1 = 0
                din_sample_offset = +clock_half_period / 4
                if first_iteration:
                    print("Slave bit clock inverted")
                    print(
                        "Data-in sampling point offset to simulate real setup/hold timing"
                    )

            rx_word = [0, 0, 0, 0]
            tx_word = [0, 0, 0, 0]
            tx_data = [
                [1, 2, 3, 4, 5, 6, 7, 8],
                [101, 102, 103, 104, 105, 106, 107, 108],
                [201, 202, 203, 204, 205, 206, 207, 208],
                [301, 302, 303, 304, 305, 306, 307, 308],
                [401, 402, 403, 404, 405, 406, 407, 408],
                [501, 502, 503, 504, 505, 506, 507, 508],
                [601, 602, 603, 604, 605, 606, 607, 608],
                [701, 702, 703, 704, 705, 706, 707, 708],
            ]
            rx_data = [
                [1, 2, 3, 4, 5, 6, 7, 8],
                [101, 102, 103, 104, 105, 106, 107, 108],
                [201, 202, 203, 204, 205, 206, 207, 208],
                [301, 302, 303, 304, 305, 306, 307, 308],
                [401, 402, 403, 404, 405, 406, 407, 408],
                [501, 502, 503, 504, 505, 506, 507, 508],
                [601, 602, 603, 604, 605, 606, 607, 608],
                [701, 702, 703, 704, 705, 706, 707, 708],
            ]

            # there is one frame lead in for the slave to sync to
            time = float(xsi.get_time())

            lr_counter = 32 + 16 + (is_i2s_justified)
            for i in range(0, 16):
                xsi.drive_port_pins(self._lrclk, lr_counter >= 32)
                lr_counter = (lr_counter + 1) & 0x3F
                xsi.drive_port_pins(self._bclk, bclk0)
                time = self.wait_until_ret(time + clock_half_period)
                xsi.drive_port_pins(self._bclk, bclk1)
                time = self.wait_until_ret(time + clock_half_period)

            for i in range(0, 32):
                xsi.drive_port_pins(self._lrclk, lr_counter >= 32)
                lr_counter = (lr_counter + 1) & 0x3F
                xsi.drive_port_pins(self._bclk, bclk0)
                time = self.wait_until_ret(time + clock_half_period)
                xsi.drive_port_pins(self._bclk, bclk1)
                time = self.wait_until_ret(time + clock_half_period)

            for i in range(0, 32):
                xsi.drive_port_pins(self._lrclk, lr_counter >= 32)
                lr_counter = (lr_counter + 1) & 0x3F
                xsi.drive_port_pins(self._bclk, bclk0)
                time = self.wait_until_ret(time + clock_half_period)
                xsi.drive_port_pins(self._bclk, bclk1)
                time = self.wait_until_ret(time + clock_half_period)

            error = False

            xsi.drive_port_pins(self._setup_resp_port, 0)
            for frame_count in range(0, num_frames):
                for i in range(0, 4):
                    rx_word[i] = 0
                    tx_word[i] = tx_data[i * 2][frame_count]

                for i in range(0, bits_per_word):
                    xsi.drive_port_pins(self._lrclk, lr_counter >= 32)
                    lr_counter = (lr_counter + 1) & 0x3F
                    xsi.drive_port_pins(self._bclk, bclk0)

                    for p in range(0, num_ins):
                        xsi.drive_port_pins(self._dout[p], tx_word[p] >> 31)
                        tx_word[p] = tx_word[p] << 1

                    if din_sample_offset < 0:
                        time = self.wait_until_ret(
                            time + clock_half_period + din_sample_offset
                        )
                    elif din_sample_offset > 0 or din_sample_offset == 0:
                        time = self.wait_until_ret(time + clock_half_period)
                        xsi.drive_port_pins(self._bclk, bclk1)
                        time = self.wait_until_ret(time + din_sample_offset)

                    for p in range(0, num_outs):
                        val = xsi.sample_port_pins(self._din[p])
                        rx_word[p] = (rx_word[p] << 1) + val

                    if din_sample_offset < 0:
                        time = self.wait_until_ret(time - din_sample_offset)
                        xsi.drive_port_pins(self._bclk, bclk1)
                        time = self.wait_until_ret(time + clock_half_period)
                    elif din_sample_offset > 0 or din_sample_offset == 0:
                        time = self.wait_until_ret(
                            time + clock_half_period - din_sample_offset
                        )

                for p in range(0, num_outs):
                    if rx_data[p * 2][frame_count] != rx_word[p]:
                        print(
                            f"ERROR: first half {frame_count}: actual ({rx_word[p]}) expected ({rx_data[p * 2][frame_count]}"
                        )
                        error = True

                for i in range(0, 4):
                    rx_word[i] = 0
                    tx_word[i] = tx_data[i * 2 + 1][frame_count]

                for i in range(0, bits_per_word):
                    xsi.drive_port_pins(self._lrclk, lr_counter >= 32)
                    lr_counter = (lr_counter + 1) & 0x3F

                    xsi.drive_port_pins(self._bclk, bclk0)

                    for p in range(0, num_ins):
                        xsi.drive_port_pins(self._dout[p], tx_word[p] >> 31)
                        tx_word[p] = tx_word[p] << 1

                    if din_sample_offset < 0:
                        time = self.wait_until_ret(
                            time + clock_half_period + din_sample_offset
                        )
                    elif din_sample_offset > 0 or din_sample_offset == 0:
                        time = self.wait_until_ret(time + clock_half_period)
                        xsi.drive_port_pins(self._bclk, bclk1)
                        time = self.wait_until_ret(time + din_sample_offset)

                    for p in range(0, num_outs):
                        val = xsi.sample_port_pins(self._din[p])
                        rx_word[p] = (rx_word[p] << 1) + val

                    if din_sample_offset < 0:
                        time = self.wait_until_ret(time - din_sample_offset)
                        xsi.drive_port_pins(self._bclk, bclk1)
                        time = self.wait_until_ret(time + clock_half_period)
                    elif din_sample_offset > 0 or din_sample_offset == 0:
                        time = self.wait_until_ret(
                            time + clock_half_period - din_sample_offset
                        )

                for p in range(0, num_outs):
                    if rx_data[p * 2 + 1][frame_count] != rx_word[p]:
                        print(
                            f"ERROR: second half frame {frame_count}: actual ({rx_word[p]}) expected ({rx_data[p * 2 + 1][frame_count]})"
                        )
                        error = True

            for i in range(0, 32):
                xsi.drive_port_pins(self._lrclk, lr_counter >= 32)
                lr_counter = (lr_counter + 1) & 0x3F
                xsi.drive_port_pins(self._bclk, bclk0)
                time = self.wait_until_ret(time + clock_half_period)
                xsi.drive_port_pins(self._bclk, bclk1)
                time = self.wait_until_ret(time + clock_half_period)

            xsi.drive_port_pins(self._setup_resp_port, 1)
            # send the response
            self.wait_for_port_pins_change([self._setup_strobe_port])
            xsi.drive_port_pins(self._setup_resp_port, error)
            self.wait_for_port_pins_change([self._setup_strobe_port])

            first_iteration = False
