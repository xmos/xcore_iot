#!/usr/bin/env python
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import sys
import pytest
import xml.etree.ElementTree as ET

if len(sys.argv) == 2:
    libs = ["lib_uart", "lib_i2c", "lib_i2s", "lib_spi"]
else:
    libs = sys.argv[2:]

acceptable_outcomes = ("skipped",)
result_fname = "test_results.xml"

test_libs = {lib: lib + "/" + result_fname for lib in libs}


@pytest.mark.parametrize("fname", test_libs.values(), ids=test_libs.keys())
def test_results(fname):
    tree = ET.parse(fname)
    root = tree.getroot()
    tsuite = root.find("testsuite")

    # There should at least be some tests in here. Assert that this is the case.
    assert len(tsuite) > 0

    for testcase in tsuite.iter("testcase"):
        # Test passed if there are no children
        if len(testcase) == 0:
            continue
        # Otherwise, test was either skipped, errored, or failed. The testcase
        # will have a child with a relevant tag - skipped, error, or failure.
        # If the tag is acceptable then carry on, or otherwise assert failure.
        else:
            for child in testcase:
                assert (
                    child.tag in acceptable_outcomes
                ), f"A test reports as {child.tag}, which is not accepted."
