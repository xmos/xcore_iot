WF200_FW=../../lib_soc/src/peripherals/bsp/wf200_driver/thirdparty/wfx-firmware/wfm_wf200_C0.sec
xflash --quad-spi-clock 50MHz --factory bin/app_freertos_wfm200.xe --boot-partition-size 0x100000 --data $WF200_FW

