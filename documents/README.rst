Building Documentation
======================

Prerequisites
-------------

Python Modules
^^^^^^^^^^^^^^

Install `Sphynx <https://www.sphinx-doc.org/en/master/>`_.

$ pip install sphynx

`Breathe <https://breathe.readthedocs.io/en/latest/>`_ is the bridge between Doxygen and Sphinx; taking the output from the former and making it available through some special directives in the latter. 

$ pip install breathe


Building
--------

$ make html