EXECUTABLE = bin/$(BOARD)/independent_tiles.xe
EXECUTABLE_T0 = bin/tile0/independent_tiles_0.xe
EXECUTABLE_T1 = bin/tile1/independent_tiles_1.xe

.PHONY: all clean distclean .FORCE
.FORCE:

all: $(EXECUTABLE)

clean:
	rm -rf build/$(BOARD)/tmp $(EXECUTABLE) $(EXECUTABLE_T0) $(EXECUTABLE_T1)
	make -C build/$(BOARD)/tile0 clean
	make -C build/$(BOARD)/tile1 clean

distclean:
	rm -rf build

build/$(BOARD)/tile0/Makefile:
	cmake -B build/$(BOARD)/tile0 -DTHIS_XCORE_TILE=0 -DBOARD=$(BOARD)

build/$(BOARD)/tile1/Makefile:
	cmake -B build/$(BOARD)/tile1 -DTHIS_XCORE_TILE=1 -DBOARD=$(BOARD)

$(EXECUTABLE_T0): build/$(BOARD)/tile0/Makefile .FORCE
	make -C build/$(BOARD)/tile0

$(EXECUTABLE_T1): build/$(BOARD)/tile1/Makefile .FORCE
	make -C build/$(BOARD)/tile1

$(EXECUTABLE): $(EXECUTABLE_T0) $(EXECUTABLE_T1)
	cp $(EXECUTABLE_T0) $(EXECUTABLE)
	mkdir -p build/$(BOARD)/tmp
	cd build/$(BOARD)/tmp && xobjdump --split ../../../$(EXECUTABLE_T1)
	xobjdump $(EXECUTABLE) -r 0,1,build/$(BOARD)/tmp/image_n0c1_2.elf
