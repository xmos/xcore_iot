#!/usr/bin/env python
# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import sys
import os
import platform
import time
import struct
import ctypes
import urllib.request
import json

import numpy as np
from matplotlib import pyplot
from PIL import Image

IMAGE_SHAPE = (96, 96)
INPUT_SHAPE = IMAGE_SHAPE + (3,)
INPUT_SCALE = 0.003921568859368563
INPUT_ZERO_POINT = -128
# NORM_SCALE = 127.5
# NORM_SHIFT = 1

OUTPUT_SCALE = 1 / 256
OUTPUT_ZERO_POINT = -128

def quantize(arr, scale, zero_point, dtype):
    t = np.round(np.float32(arr) / np.float32(scale)).astype(np.int32) + zero_point
    return np.clip(t, np.iinfo(dtype).min, np.iinfo(dtype).max).astype(dtype)


def dequantize(arr, scale, zero_point):
    return np.float32(arr.astype(np.int32) - np.int32(zero_point)) * np.float32(scale)


PRINT_CALLBACK = ctypes.CFUNCTYPE(
    None, ctypes.c_ulonglong, ctypes.c_uint, ctypes.c_char_p
)


class Endpoint(object):
    def __init__(self):
        tool_path = os.environ.get("XMOS_TOOL_PATH")
        ps = platform.system()
        if ps == 'Windows':
            lib_path = os.path.join(tool_path, 'lib', 'xscope_endpoint.dll')
        else:  # Darwin (aka MacOS) or Linux
            lib_path = os.path.join(tool_path, 'lib', 'xscope_endpoint.so')
        
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

    def send_blob(self, blob):
        CHUNK_SIZE = 128
        SLEEP_DURATION = 0.025

        self.ready = False

        for i in range(0, len(blob), CHUNK_SIZE):
            self.publish(blob[i : i + CHUNK_SIZE])
            time.sleep(SLEEP_DURATION)

ep = Endpoint()
raw_img = None

try:
    if ep.connect():
        print("Failed to connect")
    else:
        print("Connnected")
        image_file_path = sys.argv[1]
        print("Sending image to device: " + image_file_path)
        # Some png files have RGBA format. convert to RGB to be on the safe side
        img = Image.open(image_file_path).convert("RGB")
        img = img.resize(IMAGE_SHAPE)

        img_array = np.array(img).astype(np.float32)

        # Normalize to range 0..1. Needed for quantize function
        img_array = (img_array - img_array.min()) / img_array.ptp()

        img_array = quantize(img_array, INPUT_SCALE, INPUT_ZERO_POINT, np.uint8)
        raw_img = img_array.flatten().tobytes()

        ep.send_blob(raw_img)

        print("Running inference on image: " + image_file_path)
        while not ep.ready:
            pass


except KeyboardInterrupt:
    pass

ep.disconnect()

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
        else:
            print(line)

    print()
    prob = (max_value - OUTPUT_ZERO_POINT) * OUTPUT_SCALE * 100.0

    if max_value_index == 0:
        print(f"Not-Person   {prob:0.2f}%")
    else:
        print(f"Person   {prob:0.2f}%")

    img = np.frombuffer(raw_img, dtype=np.int8).reshape(INPUT_SHAPE)
    # np_img = np.round(
    #     (dequantize(np_img, INPUT_SCALE, INPUT_ZERO_POINT) + NORM_SHIFT) * NORM_SCALE
    # ).astype(np.uint8)

    # Show the image
    pyplot.imshow(img)
    pyplot.show()
