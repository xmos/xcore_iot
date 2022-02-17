#!/bin/sh

DURATION=365
OUTPUT="mqtt_broker_certs"

mkdir $OUTPUT

# See: https://mosquitto.org/man/mosquitto-tls-7.html

# Generate a certificate authority certificate and key
openssl genrsa -out $OUTPUT/ca.key 2048
openssl req -new -x509 -days $DURATION -key $OUTPUT/ca.key -out $OUTPUT/ca.crt -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=Eng/CN=ca/emailAddress=null"

# Generate a server key without encryption
openssl genrsa -out $OUTPUT/server.key 2048

# Generate a certificate signing request to send to the CA
openssl req -new -out $OUTPUT/server.csr -key $OUTPUT/server.key -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=Eng/CN=localhost/emailAddress=null"

# Send the CSR to the CA, or sign it with your CA key
openssl x509 -req -sha256 -in $OUTPUT/server.csr -CA $OUTPUT/ca.crt -CAkey $OUTPUT/ca.key -CAcreateserial -out $OUTPUT/server.crt -days $DURATION

# Generate a client key without encryption
openssl genrsa -out $OUTPUT/client.key 2048

# Generate a certificate signing request to send to the CA
openssl req -new -out $OUTPUT/client.csr -key $OUTPUT/client.key -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=Eng/CN=explorer/emailAddress=null"

# Send the CSR to the CA, or sign it with your CA key
openssl x509 -req -in  $OUTPUT/client.csr -CA  $OUTPUT/ca.crt -CAkey  $OUTPUT/ca.key -CAcreateserial -out  $OUTPUT/client.crt -days $DURATION
