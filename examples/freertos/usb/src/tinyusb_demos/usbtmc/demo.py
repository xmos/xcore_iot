#!/usr/bin/env python3

import usb.core
import usb.util
import usbtmc
import time
import sys

idVendor = 0xcafe
idProduct = 0x4000

print("Get instrument")
inst = usbtmc.Instrument(idVendor, idProduct)

print("Open instrument")
inst.open()
inst.timeout = 3

inst.clear()

print("Test idn")
idn = inst.ask("*idn?");
print("Got: {0}".format(idn))
assert (idn == "TinyUSB,ModelNumber,SerialNumber,FirmwareVer123456")
assert (inst.is_usb488)

print("Test echo")
longstr = "0123456789" * 10
for i in range(1,len(longstr)):
    x = longstr[0:i]
    y = inst.ask(x)
    print("Test len of {0}".format(i))
    assert(x == y), f"failed i={i}"

print("Tests complete")
