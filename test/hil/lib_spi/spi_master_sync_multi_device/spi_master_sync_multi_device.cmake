#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
set(APP_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/src)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -target=XCORE-AI-EXPLORER
)
set(APP_LINK_OPTIONS
    -report
    -target=XCORE-AI-EXPLORER
)

#**********************
# Tile Targets
#**********************
if(NOT DEFINED ENV{FULL_LOAD})
    set(FULL_LOAD 0 1)
else()
    set(FULL_LOAD $ENV{FULL_LOAD})
endif()

if(NOT DEFINED ENV{MISO_ENABLED})
    set(MISO_ENABLED 0 1)
else()
    set(MISO_ENABLED $ENV{MISO_ENABLED})
endif()

if(NOT DEFINED ENV{MOSI_ENABLED})
    set(MOSI_ENABLED 0 1)
else()
    set(MOSI_ENABLED $ENV{MOSI_ENABLED})
endif()

if(NOT DEFINED ENV{SPI_MODE})
    set(SPI_MODE 0 1 2 3)
else()
    set(SPI_MODE $ENV{SPI_MODE})
endif()

if(NOT DEFINED ENV{DIVS})
    set(DIVS 4 8 80)
else()
    set(DIVS $ENV{DIVS})
endif()

#**********************
# Setup targets
#**********************
foreach(load ${FULL_LOAD})
    foreach(miso ${MISO_ENABLED})
        foreach(mosi ${MOSI_ENABLED})
            foreach(mode ${SPI_MODE})
                foreach(div ${DIVS})
                    if(${miso} OR ${mosi})
                        set(TARGET_NAME "test_hil_spi_master_sync_multi_device_${load}_${miso}_${mosi}_${div}_${mode}")
                        add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
                        target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
                        target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
                        target_compile_definitions(${TARGET_NAME}
                            PRIVATE
                                ${APP_COMPILE_DEFINITIONS}
                                FULL_LOAD=${load}
                                MISO_ENABLED=${miso}
                                MOSI_ENABLED=${mosi}
                                MODE=${mode}
                                DIV=${div}
                        )
                        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
                        target_link_libraries(${TARGET_NAME} PUBLIC sdk::hil::lib_spi test_hil_lib_spi_master_tester)
                        target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
                        set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
                        unset(TARGET_NAME)
                    endif()
                endforeach()
            endforeach()
        endforeach()
    endforeach()
endforeach()
