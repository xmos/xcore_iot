#################
Visual Wake Words
#################

This example application implements the `Visual Wake Words <https://blog.tensorflow.org/2019/10/visual-wake-words-with-tensorflow-lite_30.html>`__ CNN architecture.  The VWW model is trained to classify images to two classes (person/not-person) and serves as a popular use-case for microcontrollers.

This example demonstrates how to receive input data using `xscope`.

*********************
Building the firmware
*********************

Make a directory for the build.

.. code-block:: console

    $ mkdir build
    $ cd build

Run cmake:

.. code-block:: console

    $ cmake ../
    $ make

To install, run:

.. code-block:: console

    $ make install

Now return back the application directory:

.. code-block:: console

    $ cd ..

Running the firmware
====================

Running with hardware

.. code-block:: console

    $ xrun --xscope-port localhost:10234 bin/vww.xe

Running with simulator

.. code-block:: console

    $ xsim --xscope "-realtime localhost:10234" bin/vww.xe

The firmware will now wait until a data is sent from a host application. Test images can be sent to the firmware using `xscope`.  Most RGB images should work.  The `test_image.py` script requires Python.  Ensure you have installed Python 3 and the XCore SDK Python requirements.

Sending a test image to the xcore.ai Explorer board using `xscope`. The `test_image.py` script can be found in the application directory:

.. code-block:: console

    $ ./test_image.py path/to/image
