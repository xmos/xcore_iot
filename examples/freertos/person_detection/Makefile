PROJECT_NAME = person_detect
EXECUTABLE = bin/${PROJECT_NAME}.xe
EXECUTABLE_T0 = bin/tile0/${PROJECT_NAME}_0.xe
EXECUTABLE_T1 = bin/tile1/${PROJECT_NAME}_1.xe
BOARD ?= XCORE-AI-EXPLORER
USE_EXTMEM ?= 0

.PHONY: all clean distclean run .FORCE
.FORCE:

all: $(EXECUTABLE)

clean:
	rm -rf build/tmp $(EXECUTABLE) $(EXECUTABLE_T0) $(EXECUTABLE_T1)
	make -C build/tile0 clean
	make -C build/tile1 clean

distclean:
	rm -rf build
	rm -rf bin

build/tile0/Makefile:
	cmake -B build/tile0 -DTHIS_XCORE_TILE=0 -DBOARD=$(BOARD) -DUSE_EXTMEM=${USE_EXTMEM}

build/tile1/Makefile:
	cmake -B build/tile1 -DTHIS_XCORE_TILE=1 -DBOARD=$(BOARD) -DUSE_EXTMEM=${USE_EXTMEM}

$(EXECUTABLE_T0): build/tile0/Makefile .FORCE
	make -C build/tile0

$(EXECUTABLE_T1): build/tile1/Makefile .FORCE
	make -C build/tile1

$(EXECUTABLE): $(EXECUTABLE_T0) $(EXECUTABLE_T1)
	cp $(EXECUTABLE_T0) $(EXECUTABLE)
	mkdir -p build/tmp
	cd build/tmp && xobjdump --split ../../$(EXECUTABLE_T1)
	xobjdump $(EXECUTABLE) -r 0,1,build/tmp/image_n0c1_2.elf

run: $(EXECUTABLE)
	xrun --xscope $(EXECUTABLE)
