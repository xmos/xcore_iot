#!/usr/bin/env python
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest

test_results_filename = "lib_i2s/test_results.csv"

def test_results():
    f = open(test_results_filename, "r")
    cnt = 0
    while 1:
        line = f.readline()
        cnt += 1

        if len(line) == 0:
            assert cnt >= 2 # should be at least 2 lines in each report
            break

        assert "FAIL" in line
