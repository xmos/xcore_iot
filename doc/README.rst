####################
Documentation Source
####################

This folder contains source files for the **XCore SDK documentation**.  The sources do not render well in GitHub or an RST viewer.  In addition, some information 
is not visible at all and some links will not be functional.

********************
Hosted Documentation
********************

TODO: Include URL for hosted documentation

**********************
Building Documentation
**********************

Instructions are given below to build the documentation.  The recommended method is using Docker, 
however, alternative instructions are provided in case using Docker in not an option.

To develop the content of this repository, it is recommended to launch a `sphinx-autobuild`
server as per the instructions below. Once started, point a web-browser at
http://127.0.0.1:8000. If running the server within a VM, remember to configure
port forwarding.

You can now edit the .rst documentation, and your web-browser content will automatically
update.

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
    
    $ make clean html linkcheck SPHINXOPTS="--keep-going"

Add `-W` to the `SPHINXOPTS` to turn warnings into errors.

**********************************
Building Documentation with Docker
**********************************

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

Clean and build documentation with link check:

.. code-block:: console

    $ docker run --user $(id -u) --rm -w /xcore_sdk/documents -v ${XCORE_SDK_PATH}:/xcore_sdk ghcr.io/xmos/doc_builder:main make clean html linkcheck SPHINXOPTS="--keep-going"

**********************
Adding a New Component
**********************

Follow the following steps to add a new component.

- Add an entry for the new component's top-level document to the appropriate TOC in the documents tree.
- If the new component uses `Doxygen`, append the appropriate path(s) to the INPUT variable in `Doxyfile`.
- If the new component includes `.rst` files that should **not** be part of the documentation build, append the appropriate 
path(s) to `exclude_patterns` in `conf.py`.