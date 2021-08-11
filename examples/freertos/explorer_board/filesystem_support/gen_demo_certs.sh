#!/bin/sh

mkdir echo_client_certs
openssl req -newkey rsa:2048 -nodes -x509 -sha256 -out echo_client_certs/server.pem -keyout echo_client_certs/server.key -days 365 -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=ENGDEMO/CN=localhost/emailAddress=null"

openssl genrsa -out echo_client_certs/client.key 2048

openssl req -new -key echo_client_certs/client.key -out echo_client_certs/client.csr -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=ENGDEMO/CN=demodevice/emailAddress=null"

openssl x509 -req -in echo_client_certs/client.csr -CA echo_client_certs/server.pem -CAkey echo_client_certs/server.key -CAcreateserial -out echo_client_certs/client.pem -days 365 -sha256

mkdir board_server_certs
openssl req -newkey rsa:2048 -nodes -x509 -sha256 -out board_server_certs/server.pem -keyout board_server_certs/server.key -days 365 -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=ENGDEMO/CN=localhost/emailAddress=null"

openssl genrsa -out board_server_certs/client.key 2048

openssl req -new -key board_server_certs/client.key -out board_server_certs/client.csr -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=ENGDEMO/CN=demodevice/emailAddress=null"

openssl x509 -req -in board_server_certs/client.csr -CA board_server_certs/server.pem -CAkey board_server_certs/server.key -CAcreateserial -out board_server_certs/client.pem -days 365 -sha256
