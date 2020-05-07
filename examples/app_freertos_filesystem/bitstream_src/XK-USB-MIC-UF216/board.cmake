# Hardware configurations
set(BOARD_COMPILE_FLAGS
        "-DXCORE200_MAB=1"
        "-lquadflash"
    )
set(BOARD_HW_TARGET
        "bitstream_src/${BOARD}/MIC-ARRAY-1V3.xn"
    )
set(BOARD_XC_SRCS
        "bitstream_src/${BOARD}/bitstream.xc"
    )
set(BOARD_C_SRCS
        "bitstream_src/${BOARD}/bitstream.c"
    )
set(BOARD_ASM_SRCS
        ""
    )
set(BOARD_INCLUDES
        "bitstream_src/${BOARD}"
    )
set(FREERTOS_PORT "XCORE200")
