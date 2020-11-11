#!/usr/bin/env python

# Copyright (c) 2020, XMOS Ltd, All rights reserved
import sys
import os
import time
import struct
import ctypes

import numpy as np
from matplotlib import pyplot

CHUCK_SIZE = 128

INPUT_SHAPE = (128, 128, 3)
INPUT_SCALE = 0.007843137718737125
INPUT_ZERO_POINT = -1
NORM_SCALE = 127.5
NORM_SHIFT = 1

OUTPUT_SCALE = 1/256
OUTPUT_ZERO_POINT = -128

OBJECT_CLASSES = [
    "hotdog",
    "not_hotdog",
]


PRINT_CALLBACK = ctypes.CFUNCTYPE(
    None, ctypes.c_ulonglong, ctypes.c_uint, ctypes.c_char_p
)


def dequantize(arr, scale, zero_point):
    return np.float32((arr.astype(np.int32) - np.int32(zero_point)) * scale)


class Endpoint(object):
    def __init__(self):
        tool_path = os.environ.get("XMOS_TOOL_PATH")
        lib_path = os.path.join(tool_path, "lib", "xscope_endpoint.so")
        self.lib_xscope = ctypes.CDLL(lib_path)

        self.ready = True
        self.lines = []

        self._print_cb = self._print_callback_func()
        self.lib_xscope.xscope_ep_set_print_cb(self._print_cb)

    def _print_callback_func(self):
        def func(timestamp, length, data):
            self.on_print(timestamp, data[0:length])

        return PRINT_CALLBACK(func)

    def on_print(self, timestamp, data):
        msg = data.decode().rstrip()

        if msg == "DONE!":
            self.ready = True
        else:
            self.lines.append(msg)

    def connect(self, hostname="localhost", port="10234"):
        return self.lib_xscope.xscope_ep_connect(hostname.encode(), port.encode())

    def disconnect(self):
        self.lib_xscope.xscope_ep_disconnect()

    def publish(self, data):
        self.ready = False

        return self.lib_xscope.xscope_ep_request_upload(
            ctypes.c_uint(len(data) + 1), ctypes.c_char_p(data)
        )


ep = Endpoint()
raw_img = None

try:
    if ep.connect():
        print("Failed to connect")
    else:
        # time.sleep(5)
        with open(sys.argv[1], "rb") as fd:
            raw_img = fd.read()
            for i in range(0, len(raw_img), CHUCK_SIZE):
                retval = ep.publish(raw_img[i : i + CHUCK_SIZE])
        while not ep.ready:
            pass

except KeyboardInterrupt:
    pass

ep.disconnect()
print("\n".join(ep.lines))

if raw_img is not None:
    max_value = -128
    max_value_index = 0
    for line in ep.lines:
        if line.startswith("Output index"):
            fields = line.split(",")
            index = int(fields[0].split("=")[1])
            value = int(fields[1].split("=")[1])
            if value >= max_value:
                max_value = value
                max_value_index = index
    print()
    prob = (max_value - OUTPUT_ZERO_POINT) * OUTPUT_SCALE * 100.0
    print(OBJECT_CLASSES[max_value_index], f"{prob:0.2f}%")

    np_img = np.frombuffer(raw_img, dtype=np.int8).reshape(INPUT_SHAPE)
    np_img = np.round(
        (dequantize(np_img, INPUT_SCALE, INPUT_ZERO_POINT) + NORM_SHIFT) * NORM_SCALE
    ).astype(np.uint8)

    # pyplot.imshow(np_img)
    # pyplot.show()
