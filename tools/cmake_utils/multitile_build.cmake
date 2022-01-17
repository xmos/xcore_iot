cmake_minimum_required(VERSION 3.20)

## create_multitile_target takes the variable name of a list containing the
## tiles that have an rtos placed on them.
macro(create_multitile_target _LIST_VAR)
    if(NOT ${_LIST_VAR})
        message(FATAL_ERROR "create_multitile_target expected a non-empty list.  Got \"${_LIST_VAR}\" containing ${${_LIST_VAR}}.")
    endif()

    if(NOT DEFINED XE_BASE_TILE)
        set(XE_BASE_TILE 0)
        message(STATUS "Using tile 0 as base tile by default")
    endif()

    if(NOT DEFINED OUTPUT_DIR)
        set(OUTPUT_DIR "bin")
        message(STATUS "Placing output binary in bin by default")
    endif()

    add_custom_target(${PROJECT_NAME}.xe
        ALL ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/${OUTPUT_DIR}
        COMMAND cp ${CMAKE_BINARY_DIR}/intermediates/${XE_BASE_TILE}.xe ${CMAKE_CURRENT_SOURCE_DIR}/${OUTPUT_DIR}/${PROJECT_NAME}.xe
        DEPENDS
            "${CMAKE_BINARY_DIR}/intermediates/${XE_BASE_TILE}.xe"
        BYPRODUCTS
            ${CMAKE_CURRENT_SOURCE_DIR}/${OUTPUT_DIR}
        COMMENT "Move tile binaries"
        VERBATIM
    )

    foreach(_TILE IN ITEMS ${${_LIST_VAR}})
        add_compile_definitions(PLATFORM_USES_TILE_${_TILE}=1)

    	message(STATUS "Create target for tile ${_TILE}")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/intermediates")

        set(_THIS_TARGET ${_TILE}.xe)
        add_executable(${_THIS_TARGET})

        target_sources(${_THIS_TARGET} PRIVATE ${APP_SOURCES})
        target_include_directories(${_THIS_TARGET} PRIVATE ${APP_INCLUDES})

        target_compile_options(${_THIS_TARGET} PRIVATE ${APP_COMPILER_FLAGS})
        target_compile_definitions(${_THIS_TARGET} PRIVATE THIS_XCORE_TILE=${_TILE})

        target_link_libraries(${_THIS_TARGET} PRIVATE ${APP_LINK_LIBRARIES})
        target_link_options(${_THIS_TARGET} PRIVATE ${APP_COMPILER_FLAGS})

        if(NOT ${_TILE} EQUAL ${XE_BASE_TILE})
            add_custom_target(TMP_TARGET_${_TILE}.xe
                ALL ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/intermediates/${_TILE}
                COMMAND xobjdump --split --split-dir ${CMAKE_BINARY_DIR}/intermediates/${_TILE} ${CMAKE_BINARY_DIR}/intermediates/${_THIS_TARGET}
                COMMAND xobjdump ${CMAKE_BINARY_DIR}/intermediates/${XE_BASE_TILE}.xe -r 0,${_TILE},${CMAKE_BINARY_DIR}/intermediates/${_TILE}/image_n0c${_TILE}_2.elf
                DEPENDS
                    "${CMAKE_BINARY_DIR}/intermediates/${_THIS_TARGET}"
                    "${CMAKE_BINARY_DIR}/intermediates/${XE_BASE_TILE}.xe"
                BYPRODUCTS
                    "${CMAKE_BINARY_DIR}/intermediates/${_TILE}"
                COMMENT "Merging tile ${_TILE} into ${XE_BASE_TILE}.xe"
                VERBATIM
            )

            add_dependencies(${PROJECT_NAME}.xe TMP_TARGET_${_TILE}.xe)
        endif()
    endforeach()
endmacro()
