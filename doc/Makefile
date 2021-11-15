# Makefile to build the Sphinx and Doxygen documentation
# for the XCore SDK
#

DOXYGEN_BUILDDIR=_doxygen
DOXYGEN_OUTPUTDIR=$(DOXYGEN_BUILDDIR)/xml
BREATHE_PROJECT=XCoreSDK

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?= -D breathe_projects.$(BREATHE_PROJECT)=$(DOXYGEN_OUTPUTDIR)
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = ../
CONFDIR       = ../documents
BUILDDIR      = _build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

clean:
	rm -rf $(DOXYGEN_BUILDDIR)
	rm -rf $(BUILDDIR)

.PHONY: help clean Makefile Doxyfile

$(DOXYGEN_OUTPUTDIR)/index.xml: Doxyfile
	doxygen

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile $(DOXYGEN_OUTPUTDIR)/index.xml
	$(SPHINXBUILD) -b $@ $(SPHINXOPTS) -c "$(CONFDIR)" "$(SOURCEDIR)" "$(BUILDDIR)" $(O)

livehtml:
	sphinx-autobuild --host 0.0.0.0 -c "$(CONFDIR)" "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
