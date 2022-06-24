#!/usr/bin/env python
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import re

test_results_filename = "testing/test_results.txt"
test_regex = r"^Tile\[(\d{1})\]\|FCore\[(\d{1})\]\|(\d+)\|TEST\|(\w{4}) QSPI_FLASH$"

def test_results():
    f = open(test_results_filename, "r")
    cnt = 0
    while 1:
        line = f.readline()

        if len(line) == 0:
            assert cnt == 2 # each tile should report PASS
            break

        p = re.match(test_regex, line)

        if p:
            cnt += 1
            assert p.group(4).find("PASS") != -1
