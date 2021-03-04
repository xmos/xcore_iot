Building Documentation
======================

Prerequisites
-------------

Doxygen
^^^^^^^^

Install `Doxygen <https://www.doxygen.nl/index.html>`_.

Installation will depend on your operating system.

On Debian based systems run:

.. code-block:: console

    $ sudo apt install doxygen

On Fedora run:

.. code-block:: console

    $ sudo dnf install doxygen


Python Modules
^^^^^^^^^^^^^^

Install `Sphinx <https://www.sphinx-doc.org/en/master/>`_.

.. code-block:: console

    $ pip install -U Sphinx

Install `Breathe <https://breathe.readthedocs.io/en/latest/>`_.

.. code-block:: console

    $ pip install -U breathe

Building
--------

.. code-block:: console

    $ make html

Building
--------

The following command will start a server at http://127.0.0.1:8000 and start watching for changes in the current directory.  When a change is detected, the documentation is rebuilt and any open browser windows are reloaded automatically. KeyboardInterrupt (ctrl+c) will stop the server.

.. code-block:: console

    $ sphinx-autobuild . _build/html