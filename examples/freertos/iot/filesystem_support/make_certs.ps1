# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

param(
    # An optional argument specifying the server's hostname/IP to connect to.
    [string]$ServerName
)

# NOTE:
# openssl must be added to the environment's PATH variable. Standalone binary
# installations exist, but this utility is often paired with other Windows
# development tools/environments such as Git, MinGW, MSYS installation.

$Duration=365
$Output = Join-Path -Path $PSScriptRoot -ChildPath "mqtt_broker_certs"
$GenerateServerFiles=$false
$GenerateClientFiles=$false

if (-not(Test-Path $Output)) {
    New-Item -ItemType Directory $Output | Out-Null
}

# See: https://mosquitto.org/man/mosquitto-tls-7.html

if (-not(Test-Path("$Output/ca.key")) -or -not(Test-Path("$Output/ca.crt"))) {
    $GenerateServerFiles=$true
    $GenerateClientFiles=$true

    # Generate a certificate authority certificate and key
    openssl genrsa -out "$Output/ca.key" 2048
    openssl req -new -x509 -days $Duration -key "$Output/ca.key" -out "$Output/ca.crt" -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=Eng/CN=ca/emailAddress=null"
}

if (-not(Test-Path("$Output/server.key")) -or -not(Test-Path("$Output/server.crt")) -or ($GenerateServerFiles)) {
    if ([string]::IsNullOrWhiteSpace($ServerName)) {
        do {
            $ServerName = Read-Host -Prompt "Enter the MQTT server's IP/hostname"
        } while ([string]::IsNullOrWhiteSpace($ServerName))
    } else {
        Write-Output "MQTT server's IP/hostname: '$ServerName'"
    }

    # Generate a server key without encryption
    openssl genrsa -out "$Output/server.key" 2048

    # Generate a certificate signing request to send to the CA
    openssl req -new -out "$Output/server.csr" -key "$Output/server.key" -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=Eng/CN=$ServerName/emailAddress=null"

    # Send the CSR to the CA, or sign it with your CA key
    openssl x509 -req -sha256 -in "$Output/server.csr" -CA "$Output/ca.crt" -CAkey "$Output/ca.key" -CAcreateserial -out "$Output/server.crt" -days $Duration
}

if (-not(Test-Path("$Output/client.key")) -or -not(Test-Path("$Output/client.crt")) -or ($GenerateClientFiles)) {
    # Generate a client key without encryption
    openssl genrsa -out "$Output/client.key" 2048

    # Generate a certificate signing request to send to the CA
    openssl req -new -out "$Output/client.csr" -key "$Output/client.key" -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=Eng/CN=explorer/emailAddress=null"

    # Send the CSR to the CA, or sign it with your CA key
    openssl x509 -req -in "$Output/client.csr" -CA "$Output/ca.crt" -CAkey "$Output/ca.key" -CAcreateserial -out "$Output/client.crt" -days $Duration
}
