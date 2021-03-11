# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.


class trace_processor(object):
    def __init__(self, record_list):
        self.record_list = record_list
        self.decoded_list = []

    def print_records(self):
        raise NotImplementedError()

    def print_decoded(self):
        raise NotImplementedError()

    def decode(self, verbose=False):
        raise NotImplementedError()

    def process(self, verbose=False):
        raise NotImplementedError()

    def sort_by_tick(self, verbose=False):
        raise NotImplementedError()
