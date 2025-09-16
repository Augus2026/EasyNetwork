#!/bin/bash

chmod +x generate_advanced_cert.sh
./generate_advanced_cert.sh easynet.com "DNS:easynet.com,DNS:*.easynet.com,IP:1.1.1.1"
cp certs/* /etc/easynet/

cp mtls_server /etc/init.d/
chmod +x /etc/init.d/mtls_server
/etc/init.d/mtls_server enable
/etc/init.d/mtls_server start

cp api_server /etc/init.d/
chmod +x /etc/init.d/api_server
/etc/init.d/api_server enable
/etc/init.d/api_server start
