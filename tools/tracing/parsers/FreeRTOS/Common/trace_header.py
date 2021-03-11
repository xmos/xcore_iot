# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.


class header_rec:
    def __init__(self, var, payload):
        self.Var = var
        if payload == None:
            self.Payload = ""
        else:
            self.Payload = payload

    def __str__(self):
        return self.Var + self.Payload
