#!/usr/bin/env python
# Copyright 2020-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import tensorflow as tf
import numpy as np

labels = [
    "airplane",
    "automobile",
    "bird",
    "cat",
    "deer",
    "dog",
    "frog",
    "horse",
    "ship",
    "truck",
]
n_lables = len(labels)

# load dataset
(x_train, y_train), (x_test, y_test) = tf.keras.datasets.cifar10.load_data()

# find first example of each label in dataset
test_indices = [-1] * n_lables
for i_label in range(n_lables):
    where = np.where(y_test == i_label)
    test_indices[i_label] = where[0][0]

# make test tensors and output
for i_label, i_test in enumerate(test_indices):
    orig_img = np.ndarray.astype(x_test[i_test], "int16")
    # make signed int8
    signed_img = np.ndarray.astype((orig_img - 128), "int8")
    # flatten
    flattened_img = signed_img.flatten()
    # output quantized
    fn = "{}.bin".format(labels[i_label])
    with open(fn, "wb") as fd:
        fd.write(flattened_img.tobytes())
