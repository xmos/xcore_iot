# Copyright (c) 2020, XMOS Ltd, All rights reserved

class record:
    def __init__(self, payload, len, time):
        self.Payload = payload
        self.Len = len
        self.Time = time

    def __str__(self):
        return self.Time + " " + self.Len + " " + self.Payload
