#!/bin/bash
# Accepts an optional argument specifying the server's hostname/IP to connect to.

DURATION=365
OUTPUT="mqtt_broker_certs"
GENERATE_SERVER_FILES=0
GENERATE_CLIENT_FILES=0

if [ ! -d "$OUTPUT" ]; then
    mkdir -p "$OUTPUT"
fi

# See: https://mosquitto.org/man/mosquitto-tls-7.html

if [ ! -f "$OUTPUT/ca.key" ] || [ ! -f "$OUTPUT/ca.crt" ]; then
    GENERATE_SERVER_FILES=1
    GENERATE_CLIENT_FILES=1

    # Generate a certificate authority certificate and key
    openssl genrsa -out "$OUTPUT/ca.key" 2048
    openssl req -new -x509 -days $DURATION -key "$OUTPUT/ca.key" -out "$OUTPUT/ca.crt" -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=Eng/CN=ca/emailAddress=null"
fi

if [ ! -f "$OUTPUT/server.key" ] || [ ! -f "$OUTPUT/server.crt" ] || [ $GENERATE_SERVER_FILES -eq 1 ]; then
    SERVERNAME=$1
    if [ -z "$SERVERNAME" ]; then
        while [ -z "$SERVERNAME" ]; do
            read -r -p "Enter the MQTT server's IP/hostname: " SERVERNAME
        done
    else
        echo "MQTT server's IP/hostname: '$SERVERNAME'"
    fi

    # Generate a server key without encryption
    openssl genrsa -out "$OUTPUT/server.key" 2048

    # Generate a certificate signing request to send to the CA
    openssl req -new -out "$OUTPUT/server.csr" -key "$OUTPUT/server.key" -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=Eng/CN=$SERVERNAME/emailAddress=null"

    # Send the CSR to the CA, or sign it with your CA key
    openssl x509 -req -sha256 -in "$OUTPUT/server.csr" -CA "$OUTPUT/ca.crt" -CAkey "$OUTPUT/ca.key" -CAcreateserial -out "$OUTPUT/server.crt" -days $DURATION
fi

if [ ! -f "$OUTPUT/client.key" ] || [ ! -f "$OUTPUT/client.crt" ] || [ $GENERATE_CLIENT_FILES -eq 1 ]; then
    # Generate a client key without encryption
    openssl genrsa -out "$OUTPUT/client.key" 2048

    # Generate a certificate signing request to send to the CA
    openssl req -new -out "$OUTPUT/client.csr" -key "$OUTPUT/client.key" -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=Eng/CN=explorer/emailAddress=null"

    # Send the CSR to the CA, or sign it with your CA key
    openssl x509 -req -in "$OUTPUT/client.csr" -CA  "$OUTPUT/ca.crt" -CAkey "$OUTPUT/ca.key" -CAcreateserial -out "$OUTPUT/client.crt" -days $DURATION
fi
