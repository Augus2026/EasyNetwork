#!/bin/bash

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