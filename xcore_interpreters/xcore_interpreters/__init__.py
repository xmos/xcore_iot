# Copyright (c) 2020, XMOS Ltd, All rights reserved
import sys
import ctypes

from typing import Dict, Any
from pathlib import Path

__PARENT_DIR = Path(__file__).parent.absolute()
if sys.platform.startswith("linux"):
    lib_path = str(__PARENT_DIR / "libs" / "linux" / "libxcore_interpreters.so")
elif sys.platform == "darwin":
    lib_path = str(__PARENT_DIR / "libs" / "macos" / "libxcore_interpreters.dylib")
else:
    raise RuntimeError("libxcore_interpreters is not supported on Windows!")

libxcore_interpreters = ctypes.cdll.LoadLibrary(lib_path)

from .interpreters import XCOREInterpreter
from .interpreters import XCOREDeviceInterpreter

from .interpreters import (
    InterpreterError,
    AllocateTensorsError,
    InvokeError,
    SetTensorError,
    GetTensorError,
    ModelSizeError,
    ArenaSizeError,
    DeviceTimeoutError,
)
