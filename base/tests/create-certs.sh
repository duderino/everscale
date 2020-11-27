#!/bin/bash

set -e

export KEYSIZE=4096
export EXPIRY=1 # in days

# Private Keys

if [ ! -f ca.key ]; then
	openssl genrsa -out ca.key $KEYSIZE
fi

if [ ! -f client.key ]; then
	openssl genrsa -out client.key $KEYSIZE
fi

if [ ! -f server.key ]; then
	openssl genrsa -out server.key $KEYSIZE
fi

# Create CA

openssl req -x509 -new -nodes -key ca.key -sha256 -days $EXPIRY -out ca.crt -subj '/CN=ca.everscale.com'

cat << EOF > server.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = *.server.everscale.com
EOF

cat << EOF > client.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = *.client.everscale.com
EOF

# Create CSRs

openssl req -new -key server.key -out server.csr -subj '/CN=server.everscale.com'
openssl req -new -key client.key -out client.csr -subj '/CN=client.everscale.com'

# Create Certs

openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days $EXPIRY -sha256 -extfile server.ext
openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days $EXPIRY -sha256 -extfile client.ext

# Dump Certs

#openssl x509 -in ca.crt -text
#openssl x509 -in server.crt -text
#openssl x509 -in client.crt -text

# Clean up intermediate artifacts

rm *.csr *.ext
