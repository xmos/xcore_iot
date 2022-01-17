########
Versions
########

********
Releases
********

The ``develop`` branch where new development takes place. For production use, there are also stable releases available.  The ``main`` branch represents the most recent stable release.  

A history of all releases can be found on the `Releases <https://github.com/xmos/xcore_sdk/releases>`_ page. The Releases page is where 
you can find release notes, links to each version of the documentation, and instructions for checking out each version.

**********
Versioning
**********

The XCore SDK uses `Semantic Versioning <http://semver.org/>`_.

Major Releases
==============

Major releases, like ``v1.0``, add new functionality and may change existing functionality. If updating to a new major release (for example, from ``v1.1`` to ``v2.0``), some of your application's code may need updating and functionality may need to be re-tested. See the release notes for a list of breaking changes.

Minor Releases
==============

Minor releases like ``v1.1`` add new functionality and fix bugs but will not change documented functionality and will not make incompatible changes to public APIs.  If updating to a new minor release (for example, from ``v1.0`` to ``v1.1``), your application's code does not require updating, but you should re-test your application. See the release notes for a list of important changes.

Bugfix Releases
===============

Bugfix releases like ``v1.1.1`` only fix bugs and do not add new functionality. If updating to a new bugfix release (for example, from ``v1.1`` to ``v1.1.1``), you do not need to change any code in your application, and you only need to re-test the functionality directly related to bugs listed in the release notes.

**********************************
Which Version Should I Start With?
**********************************

For production purposes, use the most recent stable release. Stable releases have been verified and are updated with "bugfix releases" when necessary. Every stable release version can be found on the `Releases`_  page.

For prototyping, experimentation or for developing new SDK features, use the ``develop`` branch. The latest version in the ``develop`` branch has all the latest features and has passed automated testing, but has not been completely verified.

**************************************
How Do I Checking the Current Version?
**************************************

The ``settings.json`` file in the root of the repository contains the version string for the current version.

``git status`` can be used to report if your local repository has commits that have not been pushed to the remote repository.  If you have local commits then you are not on a stable version.

.. code-block:: console

    $ git status -sb

**************
Support Period
**************

Each XCore SDK release version has an associated support period. After this period, the release is ``End of Life`` and no longer supported. Given the number of releases of the XCore SDK is currently small, no official support period policy exists.  Version 1.x will be support for at least two years.
