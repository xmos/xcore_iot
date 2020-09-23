# Copyright (c) 2020, XMOS Ltd, All rights reserved

class header_rec:
    def __init__(self, var, payload):
        self.Var = var
        if(payload == None):
            self.Payload = ""
        else:
            self.Payload = payload

    def __str__(self):
        return self.Var + self.Payload
