# Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
# XMOS Public License: Version 1

import pytest


def test_import():
    import xcore_interpreters

    # NOTE: pass is intentional here, just testing that import above works
    pass


if __name__ == "__main__":
    pytest.main()
