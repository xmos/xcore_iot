#=============================================================================
# Library creator shall specify the following variables:
#   LIB_NAME                Name of the library
#                           May only contain alphanumeric and space, dash, and
#                           underscore characters.
#                           ex: set(LIB_NAME lib_foo)
#   LIB_VERSION             Version of the library
#                           Must follow semantic version format.
#                           ex: set(LIB_VERSION 1.0.0)
#   LIB_ADD_COMPILER_FLAGS  Additional compiler flags
#                           ex: set(LIB_ADD_COMPILER_FLAGS "-oS")
#   LIB_XC_SRCS             List of all library XC sources
#                           Shall be set to "" if no XC sources exist
#                           ex: set(LIB_XC_SRCS "foo.xc" "bar.xc")
#   LIB_C_SRCS              List of all library C sources
#                           Shall be set to "" if no C sources exist
#                           ex: set(LIB_C_SRCS "foo.c" "bar.c")
#   LIB_ASM_SRCS            List of all library ASM sources
#                           Shall be set to "" if no ASM sources exist
#                           ex: set(LIB_ASM_SRCS "foo.S" "bar.S")
#   LIB_INCLUDES            List of all library include directories
#                           Shall be set to all directories include .h files
#                           ex: set(LIB_INCLUDES "foo/bar")
#   LIB_DEPENDENT_MODULES   List of all dependency libraries with version
#                           Every list item shall include a dependency name,
#                           which must also be the folder in which the library
#                           CMakeLists.txt exists, and a version requirement.
#                           Version requirements shall be in the format:
#                           ([bool req][major].[minor].[patch])
#                           ex: set(LIB_DEPENDENT_MODULES "lib_foo(>=1.0.0)")
#   LIB_OPTIONAL_HEADERS    List of optional header files
#                           Must contain full header file names
#                           ex: set(LIB_OPTIONAL_HEADERS "foo.h" "bar.h")
set(LIB_NAME mqtt)
set(LIB_VERSION 1.0.0)
set(LIB_ADD_COMPILER_FLAGS "")
set(LIB_XC_SRCS
        ""
    )
set(LIB_C_SRCS
        #"paho.mqtt.embedded-c/MQTTClient-C/samples/FreeRTOS/MQTTEcho.c"
        "paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectClient.c"
        #"paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectServer.c"
        "paho.mqtt.embedded-c/MQTTPacket/src/MQTTDeserializePublish.c"
        "paho.mqtt.embedded-c/MQTTPacket/src/MQTTFormat.c"
        "paho.mqtt.embedded-c/MQTTPacket/src/MQTTPacket.c"
        "paho.mqtt.embedded-c/MQTTPacket/src/MQTTSerializePublish.c"
        "paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeClient.c"
        #"paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeServer.c"
        "paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeClient.c"
        #"paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeServer.c"
        "paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.c"
        #"paho.mqtt.embedded-c/MQTTClient-C/src/FreeRTOS/MQTTFreeRTOS.c"
        "port/paho.mqtt.embedded-c/MQTTFreeRTOS.c"
    )
set(LIB_ASM_SRCS
        ""
    )
set(LIB_INCLUDES
        "paho.mqtt.embedded-c/MQTTPacket/src"
        "paho.mqtt.embedded-c/MQTTClient-C/src"
        #"paho.mqtt.embedded-c/MQTTClient-C/src/FreeRTOS"
        "port/paho.mqtt.embedded-c"
    )
set(LIB_DEPENDENT_MODULES "FreeRTOS(>=10.3.0)")
set(LIB_OPTIONAL_HEADERS "")

set(${LIB_NAME}_FILEPATH_PREFIX "${XMOS_MODULES_ROOT_DIR}/thirdparty/")

list(TRANSFORM LIB_XC_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_C_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_ASM_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_INCLUDES PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})

# #=============================================================================
# # The following part of the CMakeLists includes the common build infrastructure
# # for compiling XMOS applications. You should not need to edit below here.
XMOS_REGISTER_MODULE()
