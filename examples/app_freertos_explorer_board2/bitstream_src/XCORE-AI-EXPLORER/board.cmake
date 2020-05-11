# Hardware configurations
set(BOARD_COMPILE_FLAGS
        "-DXCOREAI_EXPLORER=1"
        "-lquadspi"
    )
set(BOARD_HW_TARGET
        "bitstream_src/${BOARD}/XCORE-AI-EXPLORER.xn"
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
set(FREERTOS_PORT "XCOREAI")
