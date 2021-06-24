# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import re
import matplotlib

regex_payload = "(\d+)\:(\d+)\:(\d+)\:(IN|OUT)\:(.+)"

class trace_processor:
    def __init__(self, list):
        self.ParsedList = list
        self.ProcessedList = [[[] for _ in range(8)] for _ in range(4)]

    def parse_payload(self):
        for each in self.ParsedList:
            p = re.match(regex_payload, each["payload"])

            if p:
                rec = {"rectime": p.group(3), "switch": p.group(4), "name": p.group(5)}
                tile = int(p.group(1))
                core = int(p.group(2))
                self.ProcessedList[tile][core].append(rec)

    def print_records(self):
        for rec in self.ProcessedList:
            print(rec)

    def get_processed(self, tile):
        return self.ProcessedList[tile]
