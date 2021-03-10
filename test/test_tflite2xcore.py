# Copyright 2020 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public License: Version 1.

import pytest


def test_third_party_dependencies():
    import tensorflow as tf
    import numpy as np
    from matplotlib import pyplot as plt

    # NOTE: pass is intentional here, just testing that import above works
    pass


def test_import():
    import tflite2xcore

    # NOTE: pass is intentional here, just testing that import above works
    pass


if __name__ == "__main__":
    pytest.main()
