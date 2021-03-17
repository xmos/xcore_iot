# Model Generation

## Generating Quantized Models

Download data for VWW quantization

    $ wget https://www.silabs.com/public/files/github/machine_learning/benchmarks/datasets/vw_coco2014_96.tar.gz
    $ tar -xvf vw_coco2014_96.tar.gz

Generate the quantized models

    $ ./make_quant_models.py

## Generating XCore Models

The `tflite2xcore` Python package environment must be installed to run the commands below.

For inference using 1 thread, run:

    $ xformer.py --analyze --num_threads 1 cifar10_quant.tflite cifar10_xcore_par1.tflite
    $ xformer.py --analyze --num_threads 1 vww_96_quant.tflite vww_96_xcore_par1.tflite

For inference using 5 threads, run:

    $ xformer.py --analyze --num_threads 5 cifar10_quant.tflite cifar10_xcore_par5.tflite
    $ xformer.py --analyze --num_threads 5 vww_96_quant.tflite vww_96_xcore_par5.tflite
