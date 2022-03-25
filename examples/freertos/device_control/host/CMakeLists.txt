set(APP_SOURCES
    "device_control_host.c"
    "commands.c"
    "argtable/argtable3.c"
)

set(APP_INCLUDES "")

add_executable(example_freertos_device_control_host)

target_link_libraries(example_freertos_device_control_host PUBLIC sdk::device_control_host_usb)
target_sources(example_freertos_device_control_host PRIVATE ${APP_SOURCES})
target_include_directories(example_freertos_device_control_host PRIVATE ${APP_INCLUDES})

message(STATUS "Building with ${CMAKE_C_COMPILER_ID}")

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	# TODO: Currently building with default compile options
	# These were originally here:
    # target_compile_options(example_freertos_device_control_host PRIVATE /O2 /W4)
	
	# These are Windows args used in app_xvf3610/host/dfu_control:
	# target_compile_options(example_freertos_device_control_host PUBLIC /RTC1 /nologo /W4 /WX- /O2 /EHa /_CRT_SECURE_NO_WARNINGS)

	target_link_options(example_freertos_device_control_host PRIVATE "")
else ()
    target_compile_options(example_freertos_device_control_host PRIVATE -O2 -Wall)
    target_link_options(example_freertos_device_control_host PRIVATE "")
endif ()
