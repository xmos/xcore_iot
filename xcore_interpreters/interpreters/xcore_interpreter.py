# Copyright (c) 2018-2019, XMOS Ltd, All rights reserved

import ctypes
import numpy as np
from enum import Enum
from typing import Sequence

from xcore_interpreters import libxcore_interpreters as lib

from . import MAX_TENSOR_ARENA_SIZE, _TensorDetails


class XCOREInterpreterStatus(Enum):
    OK = 0
    ERROR = 1


class TfLiteType(Enum):
    # originally defined in tensorflow/tensorflow/lite/c/common.h
    kTfLiteNoType = 0
    kTfLiteFloat32 = 1
    kTfLiteInt32 = 2
    kTfLiteUInt8 = 3
    kTfLiteInt64 = 4
    kTfLiteString = 5
    kTfLiteBool = 6
    kTfLiteInt16 = 7
    kTfLiteComplex64 = 8
    kTfLiteInt8 = 9
    kTfLiteFloat16 = 10
    kTfLiteFloat64 = 11


__TfLiteType_to_numpy_dtype = {
    # TfLiteType.kTfLiteString: None,  # intentionally not supported
    # TfLiteType.kTfLiteNoType: None,  # intentionally not supported
    TfLiteType.kTfLiteFloat64: np.dtype(np.float64),
    TfLiteType.kTfLiteFloat32: np.dtype(np.float32),
    TfLiteType.kTfLiteFloat16: np.dtype(np.float16),
    TfLiteType.kTfLiteComplex64: np.dtype(np.complex64),
    TfLiteType.kTfLiteInt64: np.dtype(np.int64),
    TfLiteType.kTfLiteInt32: np.dtype(np.int32),
    TfLiteType.kTfLiteInt16: np.dtype(np.int16),
    TfLiteType.kTfLiteInt8: np.dtype(np.int8),
    TfLiteType.kTfLiteUInt8: np.dtype(np.uint8),
    TfLiteType.kTfLiteBool: np.dtype(np.bool_),
}
TfLiteType.to_numpy_dtype = lambda self: __TfLiteType_to_numpy_dtype[self]

__TfLiteType_from_numpy_dtype = {
    np.dtype(np.float64): TfLiteType.kTfLiteFloat64,
    np.dtype(np.float32): TfLiteType.kTfLiteFloat32,
    np.dtype(np.float16): TfLiteType.kTfLiteFloat16,
    np.dtype(np.complex64): TfLiteType.kTfLiteComplex64,
    np.dtype(np.int64): TfLiteType.kTfLiteInt64,
    np.dtype(np.int32): TfLiteType.kTfLiteInt32,
    np.dtype(np.int16): TfLiteType.kTfLiteInt16,
    np.dtype(np.int8): TfLiteType.kTfLiteInt8,
    np.dtype(np.uint8): TfLiteType.kTfLiteUInt8,
    np.dtype(np.bool_): TfLiteType.kTfLiteBool,
}
TfLiteType.from_numpy_dtype = lambda x: __TfLiteType_from_numpy_dtype[np.dtype(x)]


def make_op_state_capture_callback(op_states, *, inputs=True, outputs=True):
    assert isinstance(op_states, list)

    keys = []
    if inputs:
        keys.append("inputs")
    if outputs:
        keys.append("outputs")

    def _callback(interpreter, operator_details):
        try:
            op_state = op_states[operator_details["index"]]
            assert isinstance(op_state, dict)
        except IndexError:
            op_state = {}
            op_states.append(op_state)

        for key in keys:
            op_state[key] = [
                {
                    "index": tensor["index"],
                    "values": interpreter.get_tensor(tensor["index"]),
                }
                for tensor in operator_details[key]
            ]

    return _callback


class XCOREInterpreter:
    def __init__(
        self,
        model_path=None,
        model_content=None,
        max_tensor_arena_size=MAX_TENSOR_ARENA_SIZE,
    ) -> None:
        self._error_msg = ctypes.create_string_buffer(4096)

        lib.new_interpreter.restype = ctypes.c_void_p
        lib.new_interpreter.argtypes = None

        lib.delete_interpreter.restype = None
        lib.delete_interpreter.argtypes = [ctypes.c_void_p]

        lib.initialize.restype = ctypes.c_int
        lib.initialize.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            ctypes.c_size_t,
        ]

        lib.allocate_tensors.restype = ctypes.c_int
        lib.allocate_tensors.argtypes = [ctypes.c_void_p]

        lib.tensors_size.restype = ctypes.c_size_t
        lib.tensors_size.argtypes = [ctypes.c_void_p]

        lib.inputs_size.restype = ctypes.c_size_t
        lib.inputs_size.argtypes = [ctypes.c_void_p]

        lib.input_tensor_index.restype = ctypes.c_size_t
        lib.input_tensor_index.argtypes = [ctypes.c_void_p, ctypes.c_size_t]

        lib.outputs_size.restype = ctypes.c_size_t
        lib.outputs_size.argtypes = [ctypes.c_void_p]

        lib.output_tensor_index.restype = ctypes.c_size_t
        lib.output_tensor_index.argtypes = [ctypes.c_void_p, ctypes.c_size_t]

        lib.set_tensor.restype = ctypes.c_int
        lib.set_tensor.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_void_p,
            ctypes.c_int,
            ctypes.c_void_p,
            ctypes.c_int,
        ]

        lib.get_tensor.restype = ctypes.c_int
        lib.get_tensor.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_void_p,
            ctypes.c_int,
            ctypes.c_void_p,
            ctypes.c_int,
        ]

        lib.get_tensor_details_buffer_sizes.restype = ctypes.c_int
        lib.get_tensor_details_buffer_sizes.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_size_t),
            ctypes.POINTER(ctypes.c_size_t),
            ctypes.POINTER(ctypes.c_size_t),
        ]

        lib.get_tensor_details.restype = ctypes.c_int
        lib.get_tensor_details.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_char_p,
            ctypes.c_int,
            ctypes.POINTER(ctypes.c_int),
            ctypes.POINTER(ctypes.c_int),
            ctypes.POINTER(ctypes.c_float),
            ctypes.POINTER(ctypes.c_int32),
        ]

        lib.get_operator_details_buffer_sizes.restype = ctypes.c_int
        lib.get_operator_details_buffer_sizes.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_size_t),
            ctypes.POINTER(ctypes.c_size_t),
        ]

        lib.get_operator_details.restype = ctypes.c_int
        lib.get_operator_details.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_char_p,
            ctypes.c_int,
            ctypes.POINTER(ctypes.c_int),
            ctypes.POINTER(ctypes.c_int),
            ctypes.POINTER(ctypes.c_int),
        ]

        lib.invoke.restype = ctypes.c_int
        lib.invoke.argtypes = [ctypes.c_void_p]

        lib.get_error.restype = ctypes.c_size_t
        lib.get_error.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

        lib.get_allocations.restype = ctypes.c_size_t
        lib.get_allocations.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

        lib.arena_used_bytes.restype = ctypes.c_size_t
        lib.arena_used_bytes.argtypes = [
            ctypes.c_void_p,
        ]

        if model_path:
            with open(model_path, "rb") as fd:
                self._model_content = fd.read()
        else:
            self._model_content = model_content

        self._max_tensor_arena_size = max_tensor_arena_size
        self._is_allocated = False
        self._op_states = []

    def __enter__(self) -> None:
        self.acquire()

    def __exit__(self, exc_type, exc_value, exc_traceback) -> None:
        self.release()

    def acquire(self) -> None:
        self.obj = lib.new_interpreter()
        status = lib.initialize(
            self.obj,
            self._model_content,
            len(self._model_content),
            self._max_tensor_arena_size,
        )
        if XCOREInterpreterStatus(status) is XCOREInterpreterStatus.ERROR:
            raise RuntimeError("Unable to initialize interpreter")

    def release(self) -> None:

        lib.delete_interpreter(self.obj)

    def _verify_allocated(self) -> None:
        if not self._is_allocated:
            self.allocate_tensors()

    def _check_status(self, status) -> None:
        if XCOREInterpreterStatus(status) is XCOREInterpreterStatus.ERROR:
            lib.get_error(self.obj, self._error_msg)
            raise RuntimeError(self._error_msg.value.decode("utf-8"))

    @property
    def tensor_arena_size(self):
        self._verify_allocated()
        return lib.arena_used_bytes(self.obj)

    def get_allocations(self):
        self._verify_allocated()
        alloc_msg = ctypes.create_string_buffer(4096)
        lib.get_allocations(self.obj, alloc_msg)
        return alloc_msg.value.decode("utf-8")

    def allocate_tensors(self) -> None:
        self._op_states = []
        if self._is_allocated:
            return  # NOTE: the TFLu interpreter can not be allocated multiple times
        self._is_allocated = True
        self._check_status(lib.allocate_tensors(self.obj))

    def invoke(
        self,
        *,
        preinvoke_callback=None,
        postinvoke_callback=None,
        capture_op_states=False,
    ):
        if capture_op_states:
            # NOTE: the original callbacks are ignored
            self._op_states = []
            cb_pre = make_op_state_capture_callback(self._op_states, outputs=False)
            cb_post = make_op_state_capture_callback(self._op_states, inputs=False)
            return self.invoke(preinvoke_callback=cb_pre, postinvoke_callback=cb_post)

        INVOKE_CALLBACK_FUNC = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_int)

        def make_operator_details(operator_index):
            # get the dimensions of the operator inputs and outputs
            inputs_size = ctypes.c_size_t()
            outputs_size = ctypes.c_size_t()
            self._check_status(
                lib.get_operator_details_buffer_sizes(
                    self.obj,
                    operator_index,
                    ctypes.byref(inputs_size),
                    ctypes.byref(outputs_size),
                )
            )

            # get the inputs and outputs tensor indices
            operator_name_max_len = 1024
            operator_name = ctypes.create_string_buffer(operator_name_max_len)
            operator_version = ctypes.c_int()
            operator_inputs = (ctypes.c_int * inputs_size.value)()
            operator_outputs = (ctypes.c_int * outputs_size.value)()
            self._check_status(
                lib.get_operator_details(
                    self.obj,
                    operator_index,
                    operator_name,
                    operator_name_max_len,
                    ctypes.byref(operator_version),
                    operator_inputs,
                    operator_outputs,
                )
            )

            # get the details
            tensor_details = self.get_tensor_details()
            return {
                "index": operator_index,
                "name": operator_name.value.decode("utf-8"),
                "version": operator_version.value,
                "inputs": [
                    tensor_details[input_index] for input_index in operator_inputs
                ],
                "outputs": [
                    tensor_details[output_index] for output_index in operator_outputs
                ],
            }

        def preinvoke_callback_hook(operator_index):
            preinvoke_callback(self, make_operator_details(operator_index))

        def postinvoke_callback_hook(operator_index):
            postinvoke_callback(self, make_operator_details(operator_index))

        self._verify_allocated()

        if preinvoke_callback:
            preinvoke_hook = INVOKE_CALLBACK_FUNC(preinvoke_callback_hook)
        else:
            preinvoke_hook = None

        if postinvoke_callback:
            postinvoke_hook = INVOKE_CALLBACK_FUNC(postinvoke_callback_hook)
        else:
            postinvoke_hook = None

        self._check_status(lib.invoke(self.obj, preinvoke_hook, postinvoke_hook))

    def set_tensor(self, tensor_index, value):
        self._verify_allocated()

        shape = value.ctypes.shape_as(ctypes.c_int)
        type_ = TfLiteType.from_numpy_dtype(value.dtype).value
        data = value.ctypes.data_as(ctypes.c_void_p)
        self._check_status(
            lib.set_tensor(self.obj, tensor_index, data, value.ndim, shape, type_)
        )

    def get_tensor(self, tensor_index):
        self._verify_allocated()

        tensor_details = self.get_tensor_details()[tensor_index]
        tensor = np.zeros(tensor_details["shape"], dtype=tensor_details["dtype"])
        shape = tensor.ctypes.shape_as(ctypes.c_int)
        type_ = TfLiteType.from_numpy_dtype(tensor.dtype).value
        data_ptr = tensor.ctypes.data_as(ctypes.c_void_p)
        self._check_status(
            lib.get_tensor(self.obj, tensor_index, data_ptr, tensor.ndim, shape, type_)
        )

        return tensor

    def _get_tensor_details(self, tensor_index: int) -> _TensorDetails:
        # first get the dimensions of the tensor
        shape_size = ctypes.c_size_t()
        scale_size = ctypes.c_size_t()
        zero_point_size = ctypes.c_size_t()

        self._check_status(
            lib.get_tensor_details_buffer_sizes(
                self.obj,
                tensor_index,
                ctypes.byref(shape_size),
                ctypes.byref(scale_size),
                ctypes.byref(zero_point_size),
            )
        )

        # allocate buffer for shape
        tensor_shape = (ctypes.c_int * shape_size.value)()
        tensor_name_max_len = 1024
        tensor_name = ctypes.create_string_buffer(tensor_name_max_len)
        tensor_type = ctypes.c_int()
        tensor_scale = (ctypes.c_float * scale_size.value)()
        tensor_zero_point = (ctypes.c_int32 * zero_point_size.value)()

        self._check_status(
            lib.get_tensor_details(
                self.obj,
                tensor_index,
                tensor_name,
                tensor_name_max_len,
                tensor_shape,
                ctypes.byref(tensor_type),
                tensor_scale,
                tensor_zero_point,
            )
        )
        scales = np.array(tensor_scale, dtype=np.float)
        if len(tensor_scale) == 1:
            scales = scales[0]

        zero_points = np.array(tensor_zero_point, dtype=np.int32)
        if len(tensor_scale) == 1:
            zero_points = zero_points[0]

        return {
            "index": tensor_index,
            "name": tensor_name.value.decode("utf-8"),
            "shape": np.array(tensor_shape, dtype=np.int32),
            "dtype": TfLiteType(tensor_type.value).to_numpy_dtype(),
            "quantization": (scales, zero_points),
        }

    def get_tensor_details(self) -> Sequence[_TensorDetails]:
        self._verify_allocated()
        tensor_count = lib.tensors_size(self.obj)
        return [
            self._get_tensor_details(tensor_index)
            for tensor_index in range(tensor_count)
        ]

    def get_input_details(self) -> Sequence[_TensorDetails]:
        self._verify_allocated()

        inputs_size = lib.inputs_size(self.obj)
        input_indices = [
            lib.input_tensor_index(self.obj, input_index)
            for input_index in range(inputs_size)
        ]

        return [self._get_tensor_details(idx) for idx in input_indices]

    def get_output_details(self) -> Sequence[_TensorDetails]:
        self._verify_allocated()

        outputs_size = lib.outputs_size(self.obj)
        output_indices = [
            lib.output_tensor_index(self.obj, output_index)
            for output_index in range(outputs_size)
        ]

        return [self._get_tensor_details(idx) for idx in output_indices]
