# Copyright (c) 2020, XMOS Ltd, All rights reserved


class InterpreterError(Exception):
    pass


class AllocateTensorsError(InterpreterError):
    pass


class InvokeError(InterpreterError):
    pass


class SetTensorError(InterpreterError):
    pass


class GetTensorError(InterpreterError):
    pass


class ModelSizeError(InterpreterError):
    pass


class ArenaSizeError(InterpreterError):
    pass


class DeviceTimeoutError(InterpreterError):
    pass


class GetProfilerTimesError(InterpreterError):
    pass
