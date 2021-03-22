#!/bin/bash

set -e

export KEYSIZE=4096
export EXPIRY=15330 # in days

#
# Create CA
#

if [ ! -f ca.key ]; then
	openssl genrsa -out ca.key $KEYSIZE
fi

openssl req -x509 -new -nodes -key ca.key -sha256 -days $EXPIRY -out ca.crt -subj '/CN=ca.everscale.com'

#
# Create Certs
#

cat << EOF > client.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = *.client.everscale.com
EOF

cat << EOF > server.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = *.server.everscale.com
EOF

cat << EOF > foo.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
EOF

cat << EOF > bar.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
EOF

cat << EOF > baz.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
EOF

cat << EOF > san1.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = f*.everscale.com
DNS.2 = *z.everscale.com
DNS.3 = b*r.everscale.com
IP.1 = 1.2.3.4
IP.2 = 5.6.7.8
EOF

cat << EOF > san2.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = q*ux.everscale.com
DNS.2 = c*.everscale.com
DNS.3 = *ly.everscale.com
EOF

cat << EOF > san3.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = f*.server.everscale.com
DNS.2 = *z.server.everscale.com
DNS.3 = b*r.server.everscale.com
IP.1 = 1.2.3.4
IP.2 = 5.6.7.8
EOF

for variant in client server foo bar baz san1 san2 san3
do
  if [ ! -f $variant.key ]; then
	  openssl genrsa -out $variant.key $KEYSIZE
  fi

  openssl req -new -key $variant.key -out $variant.csr -subj "/CN=$variant.everscale.com"
  openssl x509 -req -in $variant.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out $variant.crt -days $EXPIRY -sha256 -extfile $variant.ext
  openssl x509 -in $variant.crt -text
done

# Clean up intermediate artifacts

rm *.csr *.ext
