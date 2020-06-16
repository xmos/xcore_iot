mkdir certs
openssl req -newkey rsa:2048 -nodes -x509 -sha256 -out certs/server.pem -keyout certs/server.key -days 365 -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=ENGDEMO/CN=localhost/emailAddress=null"

openssl genrsa -out certs/client.key 2048

openssl req -new -key certs/client.key -out certs/client.csr -subj "/C=US/ST=NH/L=Hampton/O=XMOS/OU=ENGDEMO/CN=demodevice/emailAddress=null"

openssl x509 -req -in certs/client.csr -CA certs/server.pem -CAkey certs/server.key -CAcreateserial -out certs/client.pem -days 365 -sha256
