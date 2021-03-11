# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
from typing import Dict, Any

_TensorDetails = Dict[str, Any]

MAX_TENSOR_ARENA_SIZE = 64 * 1000000

from .xcore_interpreter import XCOREInterpreter

from .exceptions import (
    InterpreterError,
    AllocateTensorsError,
    InvokeError,
    SetTensorError,
    GetTensorError,
    ModelSizeError,
    ArenaSizeError,
)
