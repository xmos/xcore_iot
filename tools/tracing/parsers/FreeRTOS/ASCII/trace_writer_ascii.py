# Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
# XMOS Public License: Version 1

import re
import binascii

from Common.trace_writer import trace_writer

class trace_writer(trace_writer):
    def __init__(self, filename):
        super(trace_writer, self).__init__(filename)

    def write(self, trace):
        retstr = str(trace) + "\n"

        self.file.write(binascii.a2b_qp(retstr))
        self.recs_written += 1
