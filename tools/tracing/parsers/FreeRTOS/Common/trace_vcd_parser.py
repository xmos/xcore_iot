# Copyright 2020 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public License: Version 1.

import re

from Common.trace_record import record
from Common.trace_header import header_rec

regex_header = "\$(\w+)\s(.+\s*)*\$end$"
regex_enddefs = "\$enddefinitions\s(.+\s*)*\$end$"
regex_record = "#(\d+)\s\$comment\sl(\d+) (\w+) (\d)\s\$end$"


class trace_parser:
    def __init__(self):
        self.Timescale = None
        self.ProbeList = []
        self.HeaderList = []
        self.ParsedList = []
        self.Lines = 0
        self.Records = 0

    def parse_file(self, infile, verbose=False):
        f = open(infile, "r")

        cur = ""

        while 1:
            line = f.readline()
            if len(line) == 0:
                print("Parse complete, only header data found")
                return

            self.Lines += 1
            cur = cur + line

            p = re.match(regex_enddefs, cur)
            if p:
                cur = ""
                break

            p = re.match(regex_header, cur)
            if p:
                rec = header_rec(p.group(1), p.group(2))
                self.HeaderList.append(rec)
                # print(p.group(0))   # record
                # print(p.group(1))   # variable
                # print(p.group(2))   # payload
                cur = ""

        while 1:
            line = f.readline()
            if len(line) == 0:
                print("Parse complete")
                return

            self.Lines += 1
            cur = cur + line

            p = re.match(regex_record, cur)
            if p:
                rec = record(p.group(3), p.group(2), p.group(1))
                self.ParsedList.append(rec)
                # print('record ' + p.group(0))   # full record
                # print('time ' + p.group(1))     # time
                # print('len ' + p.group(2))      # len
                # print('payload ' + p.group(3))  # payload
                self.Records += 1
                cur = ""

    def print_records(self):
        for rec in self.ParsedList:
            print(rec)

    def get_parsed(self):
        return self.ParsedList
