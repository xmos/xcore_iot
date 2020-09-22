set(SW_SVC_NAME MQTT)
set(SW_SVC_ADD_COMPILER_FLAGS "")
set(SW_SVC_XC_SRCS
        "")
set(SW_SVC_C_SRCS
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectClient.c"
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTDeserializePublish.c"
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTFormat.c"
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTPacket.c"
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSerializePublish.c"
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeClient.c"
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeClient.c"
        #"src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectServer.c"
        #"src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeServer.c"
        #"src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeServer.c"
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.c"
        "src/sw_services/mqtt/port/MQTTFreeRTOS.c")
set(SW_SVC_ASM_SRCS
        "")
set(SW_SVC_INCLUDES
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTPacket/src"
        "src/sw_services/mqtt/paho.mqtt.embedded-c/MQTTClient-C/src"
        "src/sw_services/mqtt/port")
set(SW_SVC_DEPENDENT_MODULES
        "")
set(SW_SVC_OPTIONAL_HEADERS
        "soc_conf.h")
