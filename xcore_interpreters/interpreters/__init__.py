# Copyright (c) 2020, XMOS Ltd, All rights reserved
from typing import Dict, Any

_TensorDetails = Dict[str, Any]

MAX_TENSOR_ARENA_SIZE = 64 * 1000000
MAX_DEVICE_MODEL_CONTENT_SIZE = 500000
MAX_DEVICE_TENSOR_ARENA_SIZE = 215000

from .xcore_interpreter import XCOREInterpreter
from .xcore_device_interpreter import XCOREDeviceInterpreter

from .exceptions import (
    InterpreterError,
    AllocateTensorsError,
    InvokeError,
    SetTensorError,
    GetTensorError,
    ModelSizeError,
    ArenaSizeError,
    DeviceTimeoutError,
)