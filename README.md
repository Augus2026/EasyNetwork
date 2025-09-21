### vcpkg安装

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
ln -s /root/EA/vcpkg/vcpkg /usr/bin

vcpkg install uthash kcp wolfssl cjson
vcpkg export uthash kcp wolfssl cjson --raw --output-dir=temp
```

### cmake安装

```bash
#!/bin/sh
cmake -S ./ -B ./build -DCMAKE_TOOLCHAIN_FILE=/root/EA/vcpkg/temp/vcpkg-export-20250727-021314/scripts/buildsystems/vcpkg.cmake
cmake --build ./build --config Release
```

### 证书生成

```bash
openssl genrsa -out ca-key.pem 2048
openssl req -new -x509 -days 730 -key ca-key.pem -out ca-cert.pem \
  -subj "/C=US/ST=Montana/L=Bozeman/O=Sawtooth/OU=Consulting/CN=WolfSSL Root CA/emailAddress=info@wolfssl.com"

openssl genrsa -out server-key.pem 2048
openssl req -new -key server-key.pem -out server.csr \
  -subj "/C=US/ST=Montana/L=Bozeman/O=Sawtooth/OU=Consulting/CN=www.wolfssl.com/emailAddress=info@wolfssl.com"

openssl x509 -req -days 730 -in server.csr -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial -out server-cert.pem
openssl verify -CAfile ca-cert.pem server-cert.pem
```

### 一键生成高级自签名证书脚本

```bash
#!/bin/sh

# 一键生成 CA 证书和服务器证书链
set -e

# 参数设置
DOMAIN="${1:-www.wolfssl.com}"
SAN="${2:-DNS:${DOMAIN},IP:127.0.0.1}"  # 默认包含 DNS 和 IP
DAYS="${3:-730}"
OUTPUT_DIR="./certs"

# 创建输出目录
mkdir -p "$OUTPUT_DIR"

# 生成 CA 证书
echo "正在生成 CA 证书..."
openssl genrsa -out "${OUTPUT_DIR}/ca-key.pem" 2048
openssl req -new -x509 -days "$DAYS" -key "${OUTPUT_DIR}/ca-key.pem" -out "${OUTPUT_DIR}/ca-cert.pem" \
  -subj "/C=US/ST=Montana/L=Bozeman/O=Sawtooth/OU=Consulting/CN=WolfSSL Root CA/emailAddress=info@wolfssl.com"

# 生成服务器证书
echo "正在生成服务器证书（域名: $DOMAIN）..."
openssl genrsa -out "${OUTPUT_DIR}/server-key.pem" 2048

# 生成 CSR 配置文件
cat > "${OUTPUT_DIR}/server.csr.cnf" <<EOF
[req]
default_bits       = 2048
prompt             = no
default_md         = sha256
distinguished_name = req_distinguished_name
req_extensions     = req_ext

[req_distinguished_name]
C  = US
ST = Montana
L  = Bozeman
O  = Sawtooth
OU = Consulting
CN = $DOMAIN
emailAddress = info@wolfssl.com

[req_ext]
subjectAltName = $SAN
EOF

# 生成 CSR 和证书
openssl req -new -key "${OUTPUT_DIR}/server-key.pem" -out "${OUTPUT_DIR}/server.csr" \
  -config "${OUTPUT_DIR}/server.csr.cnf"

# 生成证书扩展配置文件
cat > "${OUTPUT_DIR}/server.ext.cnf" <<EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
extendedKeyUsage = serverAuth, clientAuth
subjectAltName = $SAN
EOF

# 用 CA 签发服务器证书
openssl x509 -req -days "$DAYS" -in "${OUTPUT_DIR}/server.csr" \
  -CA "${OUTPUT_DIR}/ca-cert.pem" -CAkey "${OUTPUT_DIR}/ca-key.pem" -CAcreateserial \
  -out "${OUTPUT_DIR}/server-cert.pem" -extfile "${OUTPUT_DIR}/server.ext.cnf"

# 验证证书链
echo "验证证书链..."
openssl verify -CAfile "${OUTPUT_DIR}/ca-cert.pem" "${OUTPUT_DIR}/server-cert.pem"

# 设置权限
chmod 600 "${OUTPUT_DIR}/ca-key.pem" "${OUTPUT_DIR}/server-key.pem"

echo "证书生成成功！"
echo "CA 证书: ${OUTPUT_DIR}/ca-cert.pem"
echo "CA 私钥: ${OUTPUT_DIR}/ca-key.pem"

echo "服务器证书: ${OUTPUT_DIR}/server-cert.pem"
echo "服务器私钥: ${OUTPUT_DIR}/server-key.pem"
```

```bash
chmod +x generate_advanced_cert.sh
./generate_advanced_cert.sh example.com
./generate_advanced_cert.sh example.com "DNS:easy_network.com,DNS:*.easy_network.com,IP:192.168.1.1"
```

### openwrt  mtls_server 服务脚本

```bash
#!/bin/sh /etc/rc.common
       
T=99   
STOP=01  
                                                    
start() {        
	cd /root/easy_network/mtls_server
	./mtls_server &
}       
                           
stop() {
	killall mtls_server
}

chmod +x /etc/init.d/mtls_server
/etc/init.d/mtls_server enable
/etc/init.d/mtls_server start
```

### openwrt  api_server 服务脚本

```bash
#!/bin/sh /etc/rc.common

START=99
STOP=01

start() {
    cd /root/easy_network/api_server
	./api_server &
}

stop() {
	killall api_server
}

chmod +x /etc/init.d/api_server
/etc/init.d/api_server enable
/etc/init.d/api_server start
```