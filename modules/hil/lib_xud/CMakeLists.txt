
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Source files
    file(GLOB_RECURSE LIB_C_SOURCES   lib_xud/lib_xud/src/*.c )
    file(GLOB_RECURSE LIB_CXX_SOURCES lib_xud/lib_xud/src/*.cc)
    file(GLOB_RECURSE LIB_XC_SOURCES  lib_xud/lib_xud/src/*.xc)
    file(GLOB_RECURSE LIB_ASM_SOURCES lib_xud/lib_xud/src/*.S )

    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_PidJumpTable.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_RxData.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_Token_In.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_Token_In_DI.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_Token_Out.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_Token_Out_DI.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_Token_Ping.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_Token_Setup.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_Token_Setup_DI.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/included/XUD_Token_SOF.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_CrcAddrCheck.S)
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_G_Crc.S) # Not in XS3 branch
    list(REMOVE_ITEM LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_TokenJmp.S)

    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_IoLoop.S PROPERTIES COMPILE_FLAGS "-fschedule -g0")
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_Main.xc PROPERTIES COMPILE_FLAGS "-mno-dual-issue")
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/user/client/XUD_SetDevAddr.xc PROPERTIES COMPILE_FLAGS "-mno-dual-issue")
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_DeviceAttach.xc PROPERTIES COMPILE_FLAGS "-mno-dual-issue -Wno-return-type")
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_PhyResetUser.xc PROPERTIES COMPILE_FLAGS "-mno-dual-issue") # Not in XS3 branch
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_Support.xc PROPERTIES COMPILE_FLAGS "-mno-dual-issue")
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_IOLoopCall.xc PROPERTIES COMPILE_FLAGS "-mno-dual-issue")
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_PowerSig.xc PROPERTIES COMPILE_FLAGS "-mno-dual-issue -Wno-return-type")
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_TestMode.xc PROPERTIES COMPILE_FLAGS "-mno-dual-issue")
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_GetDone.c PROPERTIES COMPILE_FLAGS "-mno-dual-issue") # Not in XS3 branch
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_SetCrcTableAddr.c PROPERTIES COMPILE_FLAGS "-mno-dual-issue")
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/lib_xud/lib_xud/src/core/XUD_HAL.xc PROPERTIES COMPILE_FLAGS "-Wno-return-type")

    ## cmake doesn't recognize .S files as assembly by default
    set_source_files_properties(LIB_ASM_SOURCES PROPERTIES LANGUAGE ASM)

    ## Assume all asm is XS3A for now
    set(XCORE_XS3A_SOURCES ${LIB_ASM_SOURCES})

    ## Set any local library compile options
    set(LIB_COMPILE_FLAGS "-O3" "-DREF_CLK_FREQ=100" "-fasm-linenum" "-fcomment-asm" "-DXUD_FULL_PIDTABLE=1")

    ## Includes files
    set(LIB_PUBLIC_INCLUDES  lib_xud/lib_xud/api lib_xud/lib_xud/src/user)
    set(LIB_PRIVATE_INCLUDES lib_xud/lib_xud/src lib_xud/lib_xud/src/core)

    ## Gather library sources
    set(LIB_PUBLIC_SOURCES   "")
    set(LIB_PRIVATE_SOURCES  ${LIB_C_SOURCES} ${LIB_CXX_SOURCES} ${LIB_XC_SOURCES})

    ## Append platform specific sources
    list(APPEND LIB_PRIVATE_SOURCES ${${CMAKE_SYSTEM_NAME}_SOURCES})

    ## Create library target
    ## INTERFACE because we depend on the board target supplied usb tile and core speed
    add_library(xcore_sdk_hil_lib_xud INTERFACE)
    target_sources(xcore_sdk_hil_lib_xud
        INTERFACE
            ${LIB_PUBLIC_SOURCES}
        INTERFACE
            ${LIB_PRIVATE_SOURCES}
    )
    target_include_directories(xcore_sdk_hil_lib_xud
        INTERFACE
            ${LIB_PUBLIC_INCLUDES}
        INTERFACE
            ${LIB_PRIVATE_INCLUDES}
    )
    target_compile_options(xcore_sdk_hil_lib_xud
        INTERFACE
            ${LIB_COMPILE_FLAGS}
    )
    target_link_libraries(xcore_sdk_hil_lib_xud
        INTERFACE
            sdk::legacy_compat
    )

    ## Create an alias
    add_library(sdk::hil::lib_xud ALIAS xcore_sdk_hil_lib_xud)
endif()
