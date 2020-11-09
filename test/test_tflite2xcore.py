# Copyright (c) 2020, XMOS Ltd, All rights reserved

import pytest

import tensorflow as tf
import numpy as np
from matplotlib import pyplot as plt

from tflite2xcore.utils import quantize_keras_model
from tflite2xcore.utils import quantize, dequantize
from tflite2xcore.utils import (
    apply_interpreter_to_examples,  # sets input tensors, invokes the interpreter, retrieves outputs
)


from tflite2xcore import tflite_visualize

from tflite2xcore.interpreters import XCOREInterpreter


def test_imports():
    # This test is here only to ensure that the imports all worked
    pass


if __name__ == "__main__":
    pytest.main()
