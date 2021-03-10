# Copyright 2020 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public License: Version 1.

import re


class trace_writer(object):
    def __init__(self, filename):
        self.filename = filename
        self.recs_written = 0
        self.file = open(filename, "wb")

    def write(self, trace):
        raise NotImplementedError()

    def cleanup(self):
        print("Wrote {0} lines".format(self.recs_written))
        self.file.close()
