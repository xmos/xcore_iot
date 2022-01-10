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

To build the documentation, run the following command in the root of the repository:

.. code-block:: console

    $ docker run --rm -t -u "$(id -u):$(id -g)" -v $(pwd):/build -e REPO:/build -e DOXYGEN_INCLUDE=/build/doc/Doxyfile.inc -e EXCLUDE_PATTERNS=/build/doc/exclude_patterns.inc -e DOXYGEN_INPUT=ignore ghcr.io/xmos/doc_builder:main

**********************
Adding a New Component
**********************

Follow the following steps to add a new component.

- Add an entry for the new component's top-level document to the appropriate TOC in the documents tree.
- If the new component uses `Doxygen`, append the appropriate path(s) to the INPUT variable in `Doxyfile.inc`.
- If the new component includes `.rst` files that should **not** be part of the documentation build, append the appropriate pattern(s) to `exclude_patterns.inc`.