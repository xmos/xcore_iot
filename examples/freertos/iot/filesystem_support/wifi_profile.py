#!/usr/bin/env python3
# Copyright 2020-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import os
import sys
import getpass
import struct

more = "y"
max_ssid_len = 32
max_pass_len = 32

s = bytearray()
out_file = "networks.dat"

if os.path.isfile(out_file):
    print(f"WiFi profile {out_file} already exists.")
    reconfig = input("Reconfigure? y/[n]: ")
    if reconfig.lower() != 'y':
        sys.exit(0)

while more.lower() == "y":
    while True:
        ssid = input("Enter the WiFi network SSID: ")
        if len(ssid) > 0 and len(ssid) <= max_ssid_len:
            break

    while True:
        password = getpass.getpass("Enter the WiFi network password: ")
        if len(password) <= max_pass_len:
            break

    security = -1
    while security < 0 or security > 2:
        security = eval(input("Enter the security (0=open, 1=WEP, 2=WPA): "))

    bssid = b"\x00\x00\x00\x00\x00\x00"

    more = input("Add another WiFi network? y/[n]: ")

    s += struct.pack(
        "<32sxB6s32sxBxxi",
        bytearray(ssid, "ascii"),
        min(len(ssid), max_ssid_len),
        bssid,
        bytearray(password, "ascii"),
        min(len(password), max_pass_len),
        security,
    )

file = open("networks.dat", "wb")
file.write(s)
file.close()
