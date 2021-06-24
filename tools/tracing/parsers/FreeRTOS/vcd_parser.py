# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import re

regex_header = "\$(\w+)\s(.+\s*)*\$end$"
regex_enddefs = "\$enddefinitions\s(.+\s*)*\$end$"
regex_record = "#(\d+)\s\$comment\sl(\d+) (\w+) (\d)\s\$end$"

class vcd_parser:
    def __init__(self):
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
                self.HeaderList.append(cur)
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
                time = p.group(1)
                payload_len = p.group(2)
                decoded_payload = bytearray.fromhex(p.group(3)).decode(encoding="utf-8")

                rec = {"xscopetime": time, "len": payload_len, "payload": decoded_payload}
                self.ParsedList.append(rec)
                self.Records += 1
                cur = ""

    def print_records(self):
        for rec in self.ParsedList:
            print(rec)

    def get_parsed(self):
        return self.ParsedList
