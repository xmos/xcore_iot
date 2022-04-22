#!/usr/bin/env python
# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import sys
import subprocess
import argparse

import xtagctl


def xrun(adapter_id, flags):
    args = ["xrun", "--adapter-id", adapter_id, "--xscope"]
    if flags.args:
        args.append("--args")
        args.append(flags.xe)
        args.extend(flags.args.split())
    else:
        args.append(flags.xe)

    cmd = " ".join(args)
    print(f"Running: {cmd}", file=sys.stdout)

    process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    stdout, stderr = process.communicate()

    if stdout:
        for line in stdout.decode().split("\n"):
            print(line, file=sys.stdout)
    if stderr:
        for line in stderr.decode().split("\n"):
            print(line, file=sys.stderr)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--xe",
        type=str,
        help="""\
        .xe file to run
        """,
    )
    parser.add_argument(
        "--args",
        type=str,
        default=None,
        help="""\
        .xe arguments
        """,
    )
    flags = parser.parse_args()

    with xtagctl.acquire("xcore_sdk_test_rig") as adapter_id:
        xrun(adapter_id, flags)
