#!/bin/bash

package_path="/root/easynet"
config_path="/etc/easynet/"
bin_path="/bin/easynet/"

rm -rf certs
rm -rf $config_path
rm -rf $bin_path
rm -f /etc/init.d/mtls_server
rm -f /etc/init.d/api_server

mkdir -p $config_path
mkdir -p $bin_path

cp -r $package_path/mtls-server/* ${bin_path}
cp -r $package_path/api-server/* ${bin_path}
cp -r $package_path/dashboard/ ${bin_path}

chmod +x ./generate_advanced_cert.sh
./generate_advanced_cert.sh easynet.com "DNS:easynet.com,DNS:*.easynet.com,IP:1.1.1.1"
cp -r certs ${config_path}

cp mtls_server /etc/init.d/
chmod +x /etc/init.d/mtls_server
/etc/init.d/mtls_server enable
/etc/init.d/mtls_server start

cp api_server /etc/init.d/
chmod +x /etc/init.d/api_server
/etc/init.d/api_server enable
/etc/init.d/api_server start