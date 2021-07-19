######################
Building Documentation
######################

Instructions are given below to build the documentation.  The recommended method is using Docker, 
however, alternative instructions are provided in case using Docker in not an option.

To develop the content of this repository, it is recommended to launch a `sphinx-autobuild`
server as per the instructions below. Once started, point a web-browser at
http://127.0.0.1:8000. If running the server within a VM, remember to configure
port forwarding.

You can now edit the .rst documentation, and your web-browser content will automatically
update.

************
Using Docker
************

=============
Prerequisites
=============

Install `Docker <https://www.docker.com/>`_.

Pull the docker container:

.. code-block:: console

    $ docker pull ghcr.io/xmos/doc_builder:main

========
Building
========

Build documentation:

.. code-block:: console

    $ docker run --user $(id -u) --rm -w /xcore_sdk/documents -v ${XCORE_SDK_PATH}:/xcore_sdk ghcr.io/xmos/doc_builder:main make html

Launch sphinx-autobuild server:

.. code-block:: console

    $ docker run --user $(id -u) --rm -w /xcore_sdk/documents -v ${XCORE_SDK_PATH}:/xcore_sdk ghcr.io/xmos/doc_builder:main make livehtml

Clean documentation:

.. code-block:: console

    $ docker run --user $(id -u) --rm -w /xcore_sdk/documents -v ${XCORE_SDK_PATH}:/xcore_sdk ghcr.io/xmos/doc_builder:main make clean

Clean and build documentation with link check:

.. code-block:: console

    $ docker run --user $(id -u) --rm -w /xcore_sdk/documents -v ${XCORE_SDK_PATH}:/xcore_sdk ghcr.io/xmos/doc_builder:main make clean html linkcheck SPHINXOPTS="-W --keep-going"

********************
Without Using Docker
********************

=============
Prerequisites
=============

Install `Doxygen <https://www.doxygen.nl/index.html>`_.

Install the required Python packages:

.. code-block:: console

    $ pip install -r requirements.txt

========
Building
========

Build documentation:

.. code-block:: console

    $ make html

Launch sphinx-autobuild server:

.. code-block:: console

    $ make livehtml

Clean documentation:

.. code-block:: console

    $ make clean

Clean and build documentation with link check:

.. code-block:: console
    
    $ make clean html linkcheck SPHINXOPTS="-W --keep-going"
