# Hardware configurations
set(BOARD_COMPILE_FLAGS
        "-DXCOREAI_EXPLORER=1"
        "-D__XS2A__=1" # Hack to get some libs to work with XS3
        "-lquadspi"
        "-mcmodel=large" # For the heap in DDR
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
