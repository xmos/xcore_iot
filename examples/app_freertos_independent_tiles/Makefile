EXECUTABLE = bin/app_freertos_independent_tiles.xe
EXECUTABLE_T0 = bin/tile0/app_freertos_independent_tiles_0.xe
EXECUTABLE_T1 = bin/tile1/app_freertos_independent_tiles_1.xe

.PHONY: all clean .FORCE
.FORCE:

all: $(EXECUTABLE)

clean:
	rm -rf build/tmp $(EXECUTABLE) $(EXECUTABLE_T0) $(EXECUTABLE_T1)
	make -C build/tile0 clean
	make -C build/tile1 clean

build/tile0/Makefile:
	cmake -B build/tile0 -DTHIS_XCORE_TILE=0

build/tile1/Makefile:
	cmake -B build/tile1 -DTHIS_XCORE_TILE=1

$(EXECUTABLE_T0): build/tile0/Makefile .FORCE
	make -C build/tile0

$(EXECUTABLE_T1): build/tile1/Makefile .FORCE
	make -C build/tile1

$(EXECUTABLE): $(EXECUTABLE_T0) $(EXECUTABLE_T1)
	cp $(EXECUTABLE_T0) $(EXECUTABLE)
	mkdir -p build/tmp
	cd build/tmp && xobjdump --split ../../$(EXECUTABLE_T1)
	xobjdump $(EXECUTABLE) -r 0,1,build/tmp/image_n0c1_2.elf
