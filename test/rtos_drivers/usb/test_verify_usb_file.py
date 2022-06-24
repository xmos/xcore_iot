#!/usr/bin/env python
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest

import os
import numpy as np
import scipy.io.wavfile

test_input_filename = "sine500hz.wav"
test_output_filename = "testing/output.wav"

# Compare the input and output wav files
# Since the microphone must start first there will be leading 0's on the data.
# The first nonzero sample index - 1 should be the start of the input wav file.

def test_results():
    assert os.path.exists(test_input_filename)   # input file missing
    assert os.path.exists(test_output_filename)  # output file missing

    in_rate, in_wav_file = scipy.io.wavfile.read(test_input_filename)
    out_rate, out_wav_file = scipy.io.wavfile.read(test_output_filename)

    assert in_rate == out_rate

    start = 0
    for i in range(len(out_wav_file)):
        if out_wav_file[i][0] != 0:
            start = i - 1
            break

    for i in range(start, len(out_wav_file) - start, 1):
        assert out_wav_file[i][0] == in_wav_file[i - start][0]
        assert out_wav_file[i][1] == in_wav_file[i - start][1]
