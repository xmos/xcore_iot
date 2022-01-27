#!/usr/bin/env python3
# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import getpass
import struct

more = "y"

s = bytearray()

while more.lower() == "y":
    ssid = input("Enter the WiFi network SSID: ")
    password = getpass.getpass("Enter the WiFi network password: ")
    security = eval(input("Enter the security (0=open, 1=WEP, 2=WPA): "))
    bssid = b"\x00\x00\x00\x00\x00\x00"

    more = input("Add another WiFi network? (y/n): ")

    s += struct.pack(
        "<32sxB6s32sxBxxi",
        bytearray(ssid, "ascii"),
        min(len(ssid), 32),
        bssid,
        bytearray(password, "ascii"),
        min(len(password), 32),
        security,
    )

file = open("networks.dat", "wb")
file.write(s)
file.close()

