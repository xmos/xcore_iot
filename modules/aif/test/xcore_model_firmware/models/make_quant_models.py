#!/usr/bin/env python
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import sys
import os

import numpy as np
import tensorflow as tf


def make_vww_quant():
    NUM_IMAGES = 10

    # load the model
    model = tf.keras.models.load_model("vww_96.h5")
    model.summary()

    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    model_float = converter.convert()

    def representative_dataset_gen():
        # load data
        dataset_dir = os.path.join("vw_coco2014_96/person")
        for idx, image_file in enumerate(os.listdir(dataset_dir)):
            if idx < NUM_IMAGES:
                full_path = os.path.join(dataset_dir, image_file)
                if os.path.isfile(full_path):
                    img = tf.keras.preprocessing.image.load_img(
                        full_path, color_mode="rgb"
                    ).resize((96, 96))
                    arr = tf.keras.preprocessing.image.img_to_array(img)
                    # scale input to [0, 1.0] like in training.
                    yield [arr.reshape(1, 96, 96, 3) / 255.0]
            else:
                return

    # quantize
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset_gen
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8
    model_quant = converter.convert()

    model_quant_path = "vww_96_quant.tflite"
    with open(model_quant_path, "wb") as fd:
        s = fd.write(model_quant)


def make_cifar10_quant():
    NUM_IMAGES = 10

    # load the model
    model = tf.keras.models.load_model("cifar10.h5")
    model.summary()

    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    model_float = converter.convert()

    def representative_dataset_gen():
        # load data
        (
            (train_images, train_labels),
            (test_images, test_labels),
        ) = tf.keras.datasets.cifar10.load_data()
        scale = tf.constant(255, dtype=tf.dtypes.float32)

        for idx, image in enumerate(test_images):
            if idx < NUM_IMAGES:
                yield [image.reshape(1, 32, 32, 3) / scale - 0.5]
            else:
                return

    # quantize
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset_gen
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8
    model_quant = converter.convert()

    model_quant_path = "cifar10_quant.tflite"
    with open(model_quant_path, "wb") as fd:
        s = fd.write(model_quant)


if __name__ == "__main__":
    make_cifar10_quant()
    make_vww_quant()
