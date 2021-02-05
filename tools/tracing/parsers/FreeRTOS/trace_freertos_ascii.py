# Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
# XMOS Public License: Version 1
import argparse
import re
import json

from Common.trace_vcd_parser import trace_parser

from ASCII.trace_processor_ascii import trace_processor
from ASCII.trace_writer_ascii import trace_writer

# example run
# python trace_to_ascii.py tracefile.txt -output_file=out.txt -config_file=trace_show.json

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("trace_file", help="Input trace file")

    parser.add_argument("-output_file", default="trace_output", help="Output trace file")
    parser.add_argument("-config_file", default="trace_show.json", help="Defines traces to be filtered")
    parser.add_argument("-cores", default=8, type=int, help="Number of FreeRTOS cores")
    parser.add_argument("--singleout", action="store_true", help="Generate single output file with all cores")

    parser.parse_args()
    args = parser.parse_args()

    return args

def main(trace_file, outfile, filter_args, cores, singleout):
    print("Loading trace: {0}\n".format(trace_file))

    print("Processing trace to text...\n")

    parser = trace_parser()
    parser.parse_file(trace_file)
    parsed = parser.get_parsed()

    processor = trace_processor(parsed, filter_args)
    processor.process()
    processor.decode()
    processor.sort_by_tick()

    if(singleout):
        writer = trace_writer(outfile)
        for each in processor.decoded_list:
            writer.write(each)
        writer.cleanup()
    else:
        for core in range(cores):
            writer = trace_writer(outfile + "_" + str(core))
            for each in processor.decoded_list:
                if(int(each.core) == core):
                    writer.write(each)
            writer.cleanup()

    print("Trace processing complete\n")

def json_to_dict(config_file):
    """ Convert the content of the given JSON file into a dictionary
    Args:
        config_file: JSON file with the value to parse
    Returns:
        dictionary with JSON data
    """

    datastore = None
    with open(config_file, "r") as f:
        input_str = f.read()
        # Remove '//' comments
        json_str = re.sub(r'//.*\n', '\n', input_str)
        datastore = json.loads(json_str)
        f.close()
    return datastore

if __name__ == "__main__":
    args = parse_arguments()

    filter_args = json_to_dict(args.config_file)

    main(args.trace_file, args.output_file, filter_args, args.cores, args.singleout)
