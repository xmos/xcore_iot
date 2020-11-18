# Copyright (c) 2018-2019, XMOS Ltd, All rights reserved
import numpy as np

from .xcore_interpreter import XCOREInterpreter
from .xcore_device import XCOREDeviceServer

from . import MAX_DEVICE_MODEL_CONTENT_SIZE, MAX_DEVICE_TENSOR_ARENA_SIZE

from .exceptions import (
    ModelSizeError,
    ArenaSizeError,
)


class XCOREDeviceInterpreter(XCOREInterpreter):
    def __init__(
        self,
        model_path=None,
        model_content=None,
        max_tensor_arena_size=MAX_DEVICE_TENSOR_ARENA_SIZE,
    ) -> None:
        # verify model content size is not too large
        if len(model_content) > MAX_DEVICE_MODEL_CONTENT_SIZE:
            raise ModelSizeError(
                f"model_content too large: {len(model_content)} "
                f"> {MAX_DEVICE_MODEL_CONTENT_SIZE} bytes"
            )

        # verify max_tensor_arena_size is not too large
        if max_tensor_arena_size > MAX_DEVICE_TENSOR_ARENA_SIZE:
            raise ArenaSizeError(
                f"max_tensor_arena_size too large: {len(max_tensor_arena_size)} "
                f"> {MAX_DEVICE_TENSOR_ARENA_SIZE} bytes"
            )

        super().__init__(model_path, model_content, max_tensor_arena_size)

        self._endpoint = None
        self._set_model = False

    def __enter__(self) -> None:
        super().__enter__()
        if not self._endpoint:
            self._endpoint = XCOREDeviceServer.acquire()

        return self

    def __exit__(self, exc_type, exc_value, exc_traceback) -> None:
        super().__exit__(exc_type, exc_value, exc_traceback)
        if self._endpoint:
            XCOREDeviceServer.release(self._endpoint)
            self._endpoint = None

    def allocate_tensors(self) -> None:
        super().allocate_tensors()

        if not self._set_model:
            # send model to device
            self._endpoint.set_model(self._model_content)
            self._set_model = True

    def invoke(
        self,
        *,
        preinvoke_callback=None,
        postinvoke_callback=None,
        capture_op_states=False,
    ):

        if preinvoke_callback != None or postinvoke_callback != None:
            raise NotImplementedError("Callbacks not implemented")

        self._endpoint.set_invoke()

    def set_tensor(self, tensor_index, value):
        self._verify_allocated()
        try:
            self._endpoint.set_tensor(tensor_index, value.tobytes())
        except Exception as ex:
            raise ex

    def get_tensor(self, tensor_index):
        self._verify_allocated()

        tensor_details = self.get_tensor_details()[tensor_index]

        buffer = self._endpoint.get_tensor(tensor_index)
        self._endpoint.clear()

        tensor = np.frombuffer(buffer, tensor_details["dtype"])
        tensor = tensor.reshape(tensor_details["shape"])

        return tensor
