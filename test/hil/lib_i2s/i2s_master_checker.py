# Copyright 2015-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
from typing import Sequence
from numbers import Number
from functools import partial

# We need to disable output buffering for this test to work on MacOS; this has
# no effect on Linux systems. Let's redefine print once to avoid putting the 
# same argument everywhere.
print = partial(print, flush=True)


class Clock(px.SimThread):

    def __init__(self, port: str) -> None:
        self._rate = 1000000000
        self._driving = True
        self._half_period = float(500000000000) / self._rate
        self._port = port

    def run(self) -> None:
        t = self.xsi.get_time()
        t += self._half_period
        while True:
            if self._driving:
                self.xsi.drive_port_pins(self._port, 0)
                t += self._half_period
                self.wait_until(t)
                self.xsi.drive_port_pins(self._port, 1)
                t += self._half_period
                self.wait_until(t)

    @property
    def rate(self) -> Number:
        return self._rate

    @rate.setter
    def rate(self, new_rate: Number) -> None:
        if new_rate == 0:
            self._driving = False
        else:
            self._driving = True
            self._half_period = float(500000000000) / new_rate
        self._rate = new_rate

class I2SMasterChecker(px.SimThread):
    """"
    This simulator thread will act as I2S master and check any transactions
    caused by the Slave.
    """

    def __init__(self,  
                 bclk: str, 
                 lrclk: str, 
                 din: Sequence[str], 
                 dout: Sequence[str], 
                 setup_strobe_port: str, 
                 setup_data_port: str, 
                 setup_resp_port: str, 
                 c: Clock, 
                 check_extra_bclk: bool = True):
        self._din = din
        self._dout = dout
        self._bclk = bclk
        self._lrclk = lrclk
        self._setup_strobe_port = setup_strobe_port
        self._setup_data_port = setup_data_port
        self._setup_resp_port = setup_resp_port
        self._clk = c
        self._check_extra_bclk = check_extra_bclk

    def print_setup(self, 
                    mclk_frequency: Number, 
                    mclk_bclk_ratio: Number, 
                    num_outs: Number, 
                    num_ins: Number, 
                    is_i2s_justified: bool,
                    prefix: str = "") -> None:
        bclk_frequency = mclk_frequency / mclk_bclk_ratio
        sr_frequency = bclk_frequency / 64

        print(f"{prefix}MCLK frequency: {mclk_frequency}, MCLK/BCLK ratio: {mclk_bclk_ratio}, BCLK frequency: {bclk_frequency},\tSample rate {sr_frequency}\tnum ins {num_outs},\tnum outs:{num_ins}, is i2s justified: {is_i2s_justified}")

    def get_setup_data(self, 
                       xsi:px.pyxsim.Xsi, 
                       setup_strobe_port: str, 
                       setup_data_port: str) -> int:
        self.wait_for_port_pins_change([setup_strobe_port])
        self.wait_for_port_pins_change([setup_strobe_port])
        return xsi.sample_port_pins(setup_data_port)

    def run(self):
        xsi: px.pyxsim.Xsi = self.xsi
        print("I2S Master Checker Started")

        while True:
            xsi.drive_port_pins(self._setup_resp_port, 0)
            strobe_val = xsi.sample_port_pins(self._setup_strobe_port)
            if strobe_val == 1:
                self.wait_for_port_pins_change([self._setup_strobe_port])

            mclk_frequency_u      = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            mclk_frequency_l      = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            mclk_bclk_ratio       = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            num_outs              = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            num_ins               = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            is_i2s_justified      = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            mclk_frequency        = (mclk_frequency_u<<16) + mclk_frequency_l

            self.print_setup(mclk_frequency, mclk_bclk_ratio, num_outs, num_ins, is_i2s_justified,prefix="CONFIG: ")

            bclk_frequency = mclk_frequency / mclk_bclk_ratio
            max_num_in_or_outs = 4
            num_test_frames = 4
            error = False
            frame_count = 0
            bit_count = 0
            word_count = 0
            words_per_frame = 2

            rx_word=[0, 0, 0, 0]
            tx_word=[0, 0, 0, 0]
            tx_data=[[ 1,   2,   3,   4,   5,   6,   7,   8],
                    [101, 102, 103, 104, 105, 106, 107, 108],
                    [201, 202, 203, 204, 205, 206, 207, 208],
                    [301, 302, 303, 304, 305, 306, 307, 308],
                    [401, 402, 403, 404, 405, 406, 407, 408],
                    [501, 502, 503, 504, 505, 506, 507, 508],
                    [601, 602, 603, 604, 605, 606, 607, 608],
                    [701, 702, 703, 704, 705, 706, 707, 708]]
            rx_data=[[ 1,   2,   3,   4,   5,   6,   7,   8],
                    [101, 102, 103, 104, 105, 106, 107, 108],
                    [201, 202, 203, 204, 205, 206, 207, 208],
                    [301, 302, 303, 304, 305, 306, 307, 308],
                    [401, 402, 403, 404, 405, 406, 407, 408],
                    [501, 502, 503, 504, 505, 506, 507, 508],
                    [601, 602, 603, 604, 605, 606, 607, 608],
                    [701, 702, 703, 704, 705, 706, 707, 708]]

            #start the master clock running
            self._clk.rate = mclk_frequency

            #for verifing the clock stability
            half_period = float(500000000000) / bclk_frequency

            for i in range(0, max_num_in_or_outs):
                rx_word[i] = 0
                tx_word[i] = tx_data[2*i+word_count][frame_count]
            lr_count = 0

            while (xsi.sample_port_pins(self._bclk) == 0):
                self.wait_for_port_pins_change([self._bclk])

            left = xsi.sample_port_pins(self._lrclk)

            #if i2s mode ignore the first bit
            if is_i2s_justified == True:
                self.wait_for_port_pins_change([self._bclk])
                self.wait_for_port_pins_change([self._bclk])
                lr_count = 1

            while frame_count < num_test_frames:
                #print "frame %d  word %d  bit %d"%(frame_count, word_count, bit_count)
                self.wait_for_port_pins_change([self._bclk])
                fall_time = xsi.get_time()

                if frame_count > 0:
                    t = fall_time - rise_time
                    if abs(t - half_period) > 4000.0:
                        if not error:
                            self.print_setup(mclk_frequency, mclk_bclk_ratio, num_outs, num_ins, is_i2s_justified, prefix="ERROR: ")
                            print(f"Timing error (falling edge): Frame: {frame_count} word: {word_count} bit: {bit_count}")
                            print(f"elapsed {t}ps, expected {half_period}ps")
                        error = True

                #drive
                for i in range(0, num_outs):
                    xsi.drive_port_pins(self._dout[i], tx_word[i]>>31)
                    tx_word[i] = tx_word[i]<<1

                self.wait_for_port_pins_change([self._bclk])

                rise_time = xsi.get_time()
                t = rise_time - fall_time
                if abs(t - half_period) > 4000.0:
                    if not error:
                        self.print_setup(mclk_frequency, mclk_bclk_ratio, num_outs, num_ins, is_i2s_justified)
                        print(f"Timing error (rising edge): Frame: {frame_count} word: {word_count} bit: {bit_count}")
                        print(f"elapsed {t}ps, expected {half_period}ps")
                    error = True

                #read
                for i in range(0, num_ins):
                    val = xsi.sample_port_pins(self._din[i])
                    rx_word[i] = (rx_word[i]<<1) + val

                #check the lr clock
                if xsi.sample_port_pins(self._lrclk) == left:
                    lr_count += 1
                else:
                    lr_count = 1
                left = xsi.sample_port_pins(self._lrclk)

                bit_count += 1
                if bit_count == 32:
                    bit_count = 0

                    if is_i2s_justified:
                        if lr_count != 1 :
                            print("Bad I2S LR")
                    else:
                        if lr_count != 32:
                            if not error:
                                self.print_setup(mclk_frequency, mclk_bclk_ratio, num_outs, num_ins, is_i2s_justified)
                                print("LR count error")
                            error = True

                    #check the rx'd word
                    for i in range(0, num_ins):
                        if is_i2s_justified:
                            chan = i * 2 + (1 - left)
                        else:
                            chan = i * 2 + left

                        if rx_data[chan][frame_count] != rx_word[i]:
                            if not error:
                                self.print_setup(mclk_frequency, mclk_bclk_ratio, num_outs, num_ins, is_i2s_justified)
                                print(f"rx error: expected:{rx_data[chan][frame_count]:08x} actual:{rx_word[i]:08x}")
                            error = True
                        rx_word[i] = 0

                    word_count += 1
                    if word_count == words_per_frame:
                        frame_count += 1
                        word_count = 0
                    if frame_count < 8:
                        for i in range(0, num_outs):
                            tx_word[i] = tx_data[2*i+word_count][frame_count]
                            
            if frame_count != num_test_frames:
                self.print_setup(mclk_frequency, mclk_bclk_ratio, num_outs, num_ins, is_i2s_justified)
                print(f"Error: word lost: {frame_count} MCLK:{mclk_frequency} ratio:{mclk_bclk_ratio}")

            xsi.drive_port_pins(self._setup_resp_port, 1)
            not_done = True
            while not_done:
                bclk_val              =  xsi.sample_port_pins(self._bclk)
                setup_strobe_port_val =  xsi.sample_port_pins(self._setup_strobe_port)

                #send the response
                self.wait_for_port_pins_change([self._setup_strobe_port, self._bclk])

                bclk_val_n              =  xsi.sample_port_pins(self._bclk)
                setup_strobe_port_val_n =  xsi.sample_port_pins(self._setup_strobe_port)

                if self._check_extra_bclk and (bclk_val_n != bclk_val):
                        if not error:
                            self.print_setup(mclk_frequency, mclk_bclk_ratio, num_outs, num_ins, is_i2s_justified)
                            print(f"Unexpected bclk edge MCLK:{mclk_frequency} ratio:{mclk_bclk_ratio}")
                        error = True

                if setup_strobe_port_val_n != setup_strobe_port_val:
                    xsi.drive_port_pins(self._setup_resp_port, error)
                    self.wait_for_port_pins_change([self._setup_strobe_port])
                    not_done = False
