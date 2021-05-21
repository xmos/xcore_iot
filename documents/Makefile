# Makefile to build the Sphinx and Doxygen documentation
# for the XCoreSDK SDK
#

DOXYGEN_OUTPUT_DIR=doxygen/xml
BREATHE_PROJECT=XCoreSDKSDK

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?= -Dbreathe_projects.$(BREATHE_PROJECT)=$(DOXYGEN_OUTPUT_DIR)
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = .
BUILDDIR      = _build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile Doxyfile

$(DOXYGEN_OUTPUT_DIR)/index.xml: Doxyfile
	doxygen

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile $(DOXYGEN_OUTPUT_DIR)/index.xml
	$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

livehtml:
	sphinx-autobuild --host 0.0.0.0 "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

