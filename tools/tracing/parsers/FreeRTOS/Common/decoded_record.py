# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.


class decoded_record:
    def __init__(self, core, trace, hwtick, args):
        self.core = core
        self.trace = trace
        self.hwtick = hwtick
        self.args = args

    def __str__(self):
        return "Core:{0}  Trace:{1}  HWTick:{2}  Args:{3}".format(
            self.core, self.trace, self.hwtick, self.args
        )
