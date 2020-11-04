Building Documentation
======================

Prerequisites
-------------

Python Modules
^^^^^^^^^^^^^^

Install `Sphynx <https://www.sphinx-doc.org/en/master/>`_.

$ pip install sphynx

Install `Recommonmark <https://recommonmark.readthedocs.io/en/latest/>`_ to add Markdown support to Sphynx.

$ pip install recommonmark

`Breathe <https://breathe.readthedocs.io/en/latest/>`_ is the bridge between Doxygen and Sphinx; taking the output from the former and making it available through some special directives in the latter. 

$ pip install breathe

Doxygen
^^^^^^^

Doxygen is optional.  Download and install `Doxygen <https://www.doxygen.nl/download.html#srcbin>`_


Building
--------

$ make html