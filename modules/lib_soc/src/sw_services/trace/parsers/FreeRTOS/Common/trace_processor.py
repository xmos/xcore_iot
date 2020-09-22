# Copyright (c) 2020, XMOS Ltd, All rights reserved

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
