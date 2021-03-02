NUM_PROCS := 4
CLOBBER_FLAG := '-c'

.DEFAULT_GOAL := help

#**************************
# xcore_interpreter targets
#**************************

.PHONY: xcore_interpreters_build
xcore_interpreters_build:
	cd xcore_interpreters/host_library && bash build.sh $(CLOBBER_FLAG)
	cd xcore_interpreters/xcore_firmware && bash build.sh $(CLOBBER_FLAG)

.PHONY: xcore_interpreters_unit_test
xcore_interpreters_unit_test:
	cd xcore_interpreters/xcore_interpreters && pytest -n $(NUM_PROCS) --junitxml=xcore_interpreters_junit.xml

.PHONY: xcore_interpreters_dist
xcore_interpreters_dist:
	cd xcore_interpreters && bash build_dist.sh

.PHONY: xcore_interpreters_dist_test
xcore_interpreters_dist_test:
	cd xcore_interpreters && bash test_dist.sh

#**************************
# ci target
#**************************

.PHONY: ci 
ci: CLOBBER_FLAG = '-c'
ci: xcore_interpreters_build \
 xcore_interpreters_unit_test \
 xcore_interpreters_dist_test
 
#**************************
# development targets
#**************************

.PHONY: submodule_update
submodule_update: 
	git submodule update --init --recursive

.PHONY: _develop
_develop: submodule_update xcore_interpreters_build

.PHONY: develop
develop: CLOBBER_FLAG=''
develop: _develop

.PHONY: clobber
clobber: CLOBBER_FLAG='-c'
clobber: _develop

.PHONY: help
help:
	@:  # This silences the "Nothing to be done for 'help'" output
	$(info usage: make [target])
	$(info )
	$(info )
	$(info primary targets:)
	$(info   develop                       Update submodules and build xcore_interpreters)
	$(info   clobber                       Update submodules and build xcore_interpreters with clobber flag enabled)
	$(info   ci                            Run continuous integration build and test (requires Conda environment))
	$(info )
	$(info xcore_interpreter targets:)
	$(info   xcore_interpreters_build      Run xcore_interpreters build)
	$(info   xcore_interpreters_unit_test  Run xcore_interpreters unit tests (requires Conda environment))
	$(info   xcore_interpreters_dist       Build xcore_interpreters distribution (requires Conda environment))
	$(info   xcore_interpreters_dist_test  Run xcore_interpreters distribution tests (requires Conda environment))
	$(info )
