#!/usr/bin/env python
# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import sys
import os
import ctypes
import time

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

PRINT_CALLBACK = ctypes.CFUNCTYPE(
    None, ctypes.c_ulonglong, ctypes.c_uint, ctypes.c_char_p
)

RECORD_CALLBACK = ctypes.CFUNCTYPE(
    None,
    ctypes.c_uint,  # id
    ctypes.c_ulonglong,  # timestamp
    ctypes.c_uint,  # length
    ctypes.c_ulonglong,  # dataval
    ctypes.POINTER(ctypes.c_char),  # data_bytes
)

REGISTER_CALLBACK = ctypes.CFUNCTYPE(
    None,
    ctypes.c_uint,  # id
    ctypes.c_uint,  # type
    ctypes.c_uint,  # r
    ctypes.c_uint,  # g
    ctypes.c_uint,  # b
    ctypes.c_char_p,  # name
    ctypes.c_char_p,  # unit
    ctypes.c_uint,  # data_type
    ctypes.c_char_p,  # data_name
)


class PersonDetectionEndpoint(object):
    def __init__(self):
        tool_path = os.environ.get("XMOS_TOOL_PATH")
        lib_path = os.path.join(tool_path, "lib", "xscope_endpoint.so")
        self.lib_xscope = ctypes.CDLL(lib_path)

        # create callbacks
        self._print_cb = self._print_callback_func()
        self.lib_xscope.xscope_ep_set_print_cb(self._print_cb)

        self._record_cb = self._record_callback_func()
        self.lib_xscope.xscope_ep_set_record_cb(self._record_cb)

        self._register_cb = self._register_callback_func()
        self.lib_xscope.xscope_ep_set_register_cb(self._register_cb)

        self._probe_info = {}  # probe id to probe info lookup.
        # probe_info includes name, units, data type, etc...

        self.last_img = None
        self.output_frame = None

    def _print_callback_func(self):
        def func(timestamp, length, data):
            self.on_print(timestamp, data[0:length])

        return PRINT_CALLBACK(func)

    def _record_callback_func(self):
        def func(id_, timestamp, length, data_val, data_bytes):
            self.on_probe(id_, timestamp, length, data_val, data_bytes)

        return RECORD_CALLBACK(func)

    def _register_callback_func(self):
        def func(id_, type_, r, g, b, name, unit, data_type, data_name):
            self._probe_info[id_] = {
                "type": type_,
                "name": name.decode("utf-8"),
                "unit": unit.decode("utf-8"),
                "data_type": data_type,
            }

        return REGISTER_CALLBACK(func)

    def clear(self):
        self.last_img = None
        self.output_frame = None

    def on_print(self, timestamp, data):
        msg = data.decode().rstrip()
        print(msg)

    def on_probe(self, id_, timestamp, length, data_val, data_bytes):
        probe = self._probe_info[id_]
        if probe["name"] == "input_image":
            tmp = np.frombuffer(data_bytes[0:length], dtype=np.uint8)
            self.last_img = np.reshape(tmp, (96, 96))

        if probe["name"] == "output_tensor":
            res = np.frombuffer(data_bytes[0:length], dtype=np.uint8)
            self.output_frame = self.last_img, res

    def get_frame(self):
        ret = self.output_frame
        self.output_frame = None
        return ret

    def result_ready(self):
        return self.output_frame != None

    def connect(self, hostname="localhost", port="10234"):
        return self.lib_xscope.xscope_ep_connect(hostname.encode(), port.encode())

    def disconnect(self):
        self.lib_xscope.xscope_ep_disconnect()


def update(i, ep, im):
    if ep.result_ready():
        img, res = ep.get_frame()
        title_str = "Person:" + str(res[0]) + " Not Person:" + str(res[1])
        if res[0] > res[1]:
            res_color = "green"
        else:
            res_color = "red"

        # set imshow outline
        for spine in im.axes.spines.values():
            spine.set_edgecolor(res_color)
        plt.title(title_str, fontsize=14, color=res_color)
        im.set_data(img)


if __name__ == "__main__":
    print("Person detection starting...")
    ep = PersonDetectionEndpoint()
    ep.connect()

    fig = plt.figure("Frame")
    plt.ion()

    ax = fig.add_subplot(1, 1, 1)
    plt.title("Waiting for data", fontsize=14)
    dft_img = np.zeros(shape=(96, 96))
    im = ax.imshow(dft_img, cmap="gray", vmin=0, vmax=255)
    im.axes.get_xaxis().set_visible(False)
    im.axes.get_yaxis().set_visible(False)

    ani = animation.FuncAnimation(fig, update, fargs=(ep, im), interval=50)
    plt.show(block=True)

    plt.ioff()
    ep.clear()
    ep.disconnect()
