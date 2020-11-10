# Copyright (c) 2020, XMOS Ltd, All rights reserved

import pytest


def test_third_party_dependencies():
    import tensorflow as tf
    import numpy as np
    from matplotlib import pyplot as plt


def test_import():
    import tflite2xcore


if __name__ == "__main__":
    pytest.main()
