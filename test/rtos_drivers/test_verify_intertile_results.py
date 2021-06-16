#!/usr/bin/env python
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import re

test_results_filename = "testing/test_results.txt"
test_regex = r"^Tile\[(\d{1})\]\|FCore\[(\d{1})\]\|(\d+)\|TEST\|(\w{4}) INTERTILE$"

def test_results():
    f = open(test_results_filename, "r")
    while 1:
        line = f.readline()

        if len(line) == 0:
            break

        p = re.match(test_regex, line)

        if p:
            assert p.group(4).find("PASS") != -1
