#!/usr/bin/env python

# Copyright (c) 2020, XMOS Ltd, All rights reserved
import sys
import os
import time
import struct
import ctypes

import numpy as np
from matplotlib import pyplot
from PIL import Image

CHUCK_SIZE = 128

IMAGE_SHAPE = (128, 128)
INPUT_SHAPE = IMAGE_SHAPE+(3,)
INPUT_SCALE = 0.007843137718737125
INPUT_ZERO_POINT = -1
NORM_SCALE = 127.5
NORM_SHIFT = 1
BATCH_RUN = 0  # Set to 1 if this script is run as part of a batch job

OUTPUT_SCALE = 1/256
OUTPUT_ZERO_POINT = -128

OBJECT_CLASSES = [
    "tench",
    "goldfish",
    "great_white_shark",
    "tiger_shark",
    "hammerhead",
    "electric_ray",
    "stingray",
    "cock",
    "hen",
    "ostrich",
]


PRINT_CALLBACK = ctypes.CFUNCTYPE(
    None, ctypes.c_ulonglong, ctypes.c_uint, ctypes.c_char_p
)


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

from tflite2xcore.utils import quantize, dequantize   

ep = Endpoint()
raw_img = None

try:
    if ep.connect():
        print("Failed to connect")
    else:
        image_file_path = sys.argv[1]
        print("Connnected")
        print("Running inference on image "+image_file_path)
        # Some png files have RGBA format. convert to RGB to be on the safe side
        img = Image.open(image_file_path).convert('RGB')
        img = img.resize(IMAGE_SHAPE)

        img_array = np.array(img).astype(np.float32)

        # Normalize to range 0..1. Needed for quantize function
        img_array = (img_array - img_array.min()) / img_array.ptp()

        img_array = quantize(img_array, INPUT_SCALE, INPUT_ZERO_POINT)   
        raw_img = img_array.flatten().tobytes()

        for i in range(0, len(raw_img), CHUCK_SIZE):
            retval = ep.publish(raw_img[i : i + CHUCK_SIZE])

        while not ep.ready:
            pass


except KeyboardInterrupt:
    pass

ep.disconnect()
print("\n".join(ep.lines))


def accumulate_times(lines):
    import sys
    import re

    total_us = 0
    for line in lines:
       m = re.search('(\d+) microseconds', line)
       if(m != None):
         total_us += int(m.group(1))
    return total_us
    
inference_time_us = accumulate_times(ep.lines)

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
    print("Time taken for inference: {} microseconds".format(inference_time_us))

    np_img = np.frombuffer(raw_img, dtype=np.int8).reshape(INPUT_SHAPE)
    np_img = np.round(
        (dequantize(np_img, INPUT_SCALE, INPUT_ZERO_POINT) + NORM_SHIFT) * NORM_SCALE
    ).astype(np.uint8)
    if not BATCH_RUN:
      # Show the image how it was processed by the model  
      pyplot.imshow(np_img)
      pyplot.show()

