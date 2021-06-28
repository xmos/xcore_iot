# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import argparse
import re
import json

from vcd_parser import vcd_parser
from trace_processor import trace_processor
from tile_grapher import tile_grapher

# example run
# python trace_to_ascii.py tracefile.vcd -output_file=out.txt

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("trace_file", help="Input trace file")
    parser.add_argument("-output_file", default="trace_output", help="Output trace file")

    parser.parse_args()
    args = parser.parse_args()

    return args

def main(trace_file, outfile):
    print("Loading trace: {0}\n".format(trace_file))

    print("Processing trace to text...\n")

    vcd = vcd_parser()
    vcd.parse_file(trace_file)

    processor = trace_processor(vcd.get_parsed())
    processor.parse_payload()

    grapher0 = tile_grapher(processor.get_processed(0), 0)
    grapher0.graph()

    grapher1 = tile_grapher(processor.get_processed(1), 1)
    grapher1.graph()


    print("Trace processing complete\n")

if __name__ == "__main__":
    args = parse_arguments()

    main(args.trace_file, args.output_file)
