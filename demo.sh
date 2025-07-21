#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=cmake-build-debug

PGW_SERVER="$BUILD_DIR/pgw_server"
PGW_CLIENT="$BUILD_DIR/pgw_client"
SERVER_CFG=server_config.json
CLIENT_CFG=client_config.json

PGW_LOG=pgw.log
CDR_LOG=cdr.log
CLIENT_LOG=client.log

cleanup() {
  pkill -f "$PGW_SERVER" &>/dev/null || true
}
trap cleanup EXIT

# Удаляем старые логи
rm -f "$PGW_LOG" "$CDR_LOG" "$CLIENT_LOG"

echo ">>> Запускаем сервер: $PGW_SERVER $SERVER_CFG"
$PGW_SERVER "$SERVER_CFG" &
SERVER_PID=$!
sleep 1

echo
echo ">>> Клиент: создаём сессию для 001010111111111"
$PGW_CLIENT "$CLIENT_CFG" 001010111111111
echo

echo ">>> Клиент: пытаемся использовать чёрный IMSI 001010123456789"
$PGW_CLIENT "$CLIENT_CFG" 001010123456789
echo

echo ">>> Клиент: повторная попытка 001010111111111"
$PGW_CLIENT "$CLIENT_CFG" 001010111111111
echo

echo ">>> HTTP /check_subscriber?imsi=001010111111111"
curl -s "http://127.0.0.1:8080/check_subscriber?imsi=001010111111111"
echo

echo ">>> HTTP /check_subscriber?imsi=001010123456789"
curl -s "http://127.0.0.1:8080/check_subscriber?imsi=001010123456789"
echo

echo ">>> Ждём 31 секунду для истечения сессии (таймаут 30s)..."
sleep 31

echo ">>> HTTP /check_subscriber?imsi=001010111111111 после таймаута"
curl -s "http://127.0.0.1:8080/check_subscriber?imsi=001010111111111"
echo

echo ">>> HTTP /stop"
curl -s "http://127.0.0.1:8080/stop"
echo

# Даем немного подгрузиться graceful shutdown
sleep 2

echo
echo "=== Содержимое $PGW_LOG ==="
cat "$PGW_LOG"
echo
echo "=== Содержимое $CDR_LOG ==="
cat "$CDR_LOG"
echo
echo "=== Содержимое $CLIENT_LOG ==="
cat "$CLIENT_LOG"

