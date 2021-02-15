Building Documentation
======================

Prerequisites
-------------

Python Modules
^^^^^^^^^^^^^^

Install `Sphynx <https://www.sphinx-doc.org/en/master/>`_.

.. code-block:: console

    $ pip install Sphynx

Building
--------

.. code-block:: console

    $ make html

Building
--------

The following command will start a server at http://127.0.0.1:8000 and start watching for changes in the current directory.  When a change is detected, the documentation is rebuilt and any open browser windows are reloaded automatically. KeyboardInterrupt (ctrl+c) will stop the server.

.. code-block:: console

    $ sphinx-autobuild . _build/html