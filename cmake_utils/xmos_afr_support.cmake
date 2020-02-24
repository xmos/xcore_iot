if(XMOS_AFR_FreeRTOS)
    list(FIND DEP_MODULE_LIST "FreeRTOS" FOUND_FRTOS)
    if(${FOUND_FRTOS} GREATER -1)
        list(REMOVE_ITEM DEP_MODULE_LIST "FreeRTOS")
        list(APPEND DEP_MODULE_LIST "AFR::kernel")
        message("Replaced dependency FreeRTOS with AFR::kernel")
    endif()
endif()

if(XMOS_AFR_FreeRTOS-Plus-TCP)
    list(FIND DEP_MODULE_LIST "FreeRTOS-Plus-TCP" FOUND_FRTOS_TCP)
    if(${FOUND_FRTOS_TCP} GREATER -1)
        list(REMOVE_ITEM DEP_MODULE_LIST "FreeRTOS-Plus-TCP")
        list(APPEND DEP_MODULE_LIST "AFR::freertos_plus_tcp")
        message("Replaced dependency FreeRTOS-Plus-TCP with AFR::freertos_plus_tcp")
    endif()
endif()



## Registers an application and it's dependencies
function(XMOS_AFR_REGISTER)
    set(XMOS_AFR_FreeRTOS True)
    set(XMOS_AFR_FreeRTOS-Plus-TCP True)
    set(FreeRTOS_SILENT_FLAG True)
    set(FreeRTOS-Plus-TCP_SILENT_FLAG True)

    if(NOT APP_HW_TARGET)
        message(FATAL_ERROR "APP_HW_TARGET is not defined.")
    endif()

    ## Populate build flag for hardware target
    if(EXISTS ${BOARD_DIR}/${APP_HW_TARGET})
        get_filename_component(HW_ABS_PATH ${BOARD_DIR}/${APP_HW_TARGET} ABSOLUTE)
        set(APP_TARGET_COMPILER_FLAG ${HW_ABS_PATH})
    else()
        set(APP_TARGET_COMPILER_FLAG -target=${APP_HW_TARGET})
    endif()

    set(LIB_NAME ${PROJECT_NAME}_LIB)
    set(LIB_VERSION ${PROJECT_VERSION})
    set(LIB_ADD_COMPILER_FLAGS ${APP_COMPILER_FLAGS})
    set(LIB_XC_SRCS ${APP_XC_SRCS})
    set(LIB_C_SRCS ${APP_C_SRCS})
    set(LIB_ASM_SRCS ${APP_ASM_SRCS})
    set(LIB_INCLUDES ${APP_INCLUDES})
    set(LIB_DEPENDENT_MODULES ${APP_DEPENDENT_MODULES})
    set(LIB_OPTIONAL_HEADERS "")
    set(LIB_FILE_FLAGS "")

    XMOS_REGISTER_MODULE("silent")

    get_target_property(${PROJECT_NAME}_LIB_SRCS ${PROJECT_NAME}_LIB SOURCES)
    get_target_property(${PROJECT_NAME}_LIB_INCS ${PROJECT_NAME}_LIB INCLUDE_DIRECTORIES)
    get_target_property(${PROJECT_NAME}_LIB_OPTINCS ${PROJECT_NAME}_LIB OPTIONAL_HEADERS)

    set(APP_SOURCES ${${PROJECT_NAME}_LIB_SRCS})
    set(APP_INCLUDES ${${PROJECT_NAME}_LIB_INCS})

    get_property(XMOS_TARGETS_LIST GLOBAL PROPERTY XMOS_TARGETS_LIST)

    foreach(lib ${XMOS_TARGETS_LIST})
        get_target_property(inc ${lib} INCLUDE_DIRECTORIES)
        list(APPEND APP_INCLUDES ${inc})
    endforeach()

    list(REMOVE_DUPLICATES APP_SOURCES)
    list(REMOVE_DUPLICATES APP_INCLUDES)

    foreach(file ${APP_SOURCES})
        get_filename_component(ext ${file} EXT)
        if(${ext} STREQUAL ".xc")
            get_filename_component(abs ${file} ABSOLUTE)
            set_source_files_properties(${abs} PROPERTIES LANGUAGE C)
        endif()
    endforeach()

    # Only define header exists, if header optional
    foreach(inc ${APP_INCLUDES})
        file(GLOB headers ${inc}/*.h)
        foreach(header ${headers})
            get_filename_component(name ${header} NAME)
            list(FIND ${PROJECT_NAME}_LIB_OPTINCS ${name} FOUND)
            if(${FOUND} GREATER -1)
                get_filename_component(name_we ${header} NAME_WE)
                list(APPEND HEADER_EXIST_FLAGS -D__${name_we}_h_exists__)
            endif()
        endforeach()
    endforeach()

    set(XMOS_APP_COMPILE_FLAGS ${APP_TARGET_COMPILER_FLAG} ${APP_COMPILER_FLAGS} ${HEADER_EXIST_FLAGS})

    foreach(target ${XMOS_TARGETS_LIST})
        target_include_directories(${target} PRIVATE ${APP_INCLUDES})
        target_compile_options(${target} BEFORE PRIVATE ${XMOS_APP_COMPILE_FLAGS})
    endforeach()

    set(DEPS_TO_LINK "")
    foreach(DEP_MODULE ${LIB_DEPENDENT_MODULES})
        string(REGEX MATCH "^[A-Za-z0-9_ -]+" DEP_NAME ${DEP_MODULE})
        list(APPEND DEPS_TO_LINK ${DEP_NAME})
    endforeach()

    set(XMOS_APP_LINK_LIBRARIES ${DEPS_TO_LINK} PARENT_SCOPE)
    set(XMOS_APP_SOURCES ${APP_SOURCES} PARENT_SCOPE)
    set(XMOS_APP_INCLUDES ${APP_INCLUDES} PARENT_SCOPE)
    set(XMOS_APP_COMPILE_FLAGS ${XMOS_APP_COMPILE_FLAGS} PARENT_SCOPE)
endfunction()
