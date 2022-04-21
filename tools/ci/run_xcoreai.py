#!/usr/bin/env python
# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import os
import xtagctl
import argparse


def xrun(adapter_id, xe):
    print(adapter_id)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--xe",
        type=str,
        help="""\
        .xe file to run
        """,
    )
    args = parser.parse_args()

    with xtagctl.acquire("6L283N7A") as adapter_id:
        xrun(adapter_id, args.xe)
