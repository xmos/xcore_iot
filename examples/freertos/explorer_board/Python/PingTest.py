# Copyright 2019 XMOS LIMITED. This Software is subject to the terms of the 
# XMOS Public License: Version 1
from __future__ import division
from __future__ import print_function

import argparse
import re
import subprocess
import time


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("ip", help="IP to ping")
    parser.add_argument("-interval", type=float, default=0.25, help="Ping interval in seconds")

    parser.parse_args()
    args = parser.parse_args()

    return args

def main(ip, interval):
    min_time = 10000;    #should never be larger than this
    max_time = 0;
    cnt = 0;
    mean_accum = 0;

    while(1):
        output = subprocess.Popen(["ping","-c 1", ip],stdout = subprocess.PIPE).communicate()[0]
        strout = output.decode('UTF-8')

        ping_time_regex = re.match(r'[.\s\S]* time=(.*) ms[.\s\S]*', strout)

        if ping_time_regex:
            cnt += 1
            ping_time = float(ping_time_regex.group(1))
            if ping_time < min_time:
                min_time = ping_time
            if ping_time > max_time:
                max_time = ping_time
            mean_accum += ping_time
            mean = round(mean_accum / cnt, 3)
            print(f"Ping Cnt:{cnt} Max:{max_time} Min:{min_time} Mean:{mean}")
        
        time.sleep(interval)
            


if __name__ == "__main__":
    args = parse_arguments()
    main(args.ip, args.interval)
