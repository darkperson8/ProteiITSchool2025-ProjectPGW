#!/usr/bin/env bash
set -euo pipefail

if [ $# -ne 1 ]; then
  echo "Usage: $0 <build_dir>"
  exit 1
fi

BUILD_DIR=$1
PGW_SERVER="$BUILD_DIR/pgw_server"
PGW_CLIENT="$BUILD_DIR/pgw_client"

SERVER_CFG="server_config.json"
CLIENT_CFG="client_config.json"
NORMAL_IMSI="001010111111111"
BLACK_IMSI="001010123456789"

PGW_LOG="pgw.log"
CDR_LOG="cdr.log"
CLIENT_LOG="client.log"

# Функция-очистка: убиваем сервер при выходе
cleanup(){
  pkill -f "$PGW_SERVER" &>/dev/null || true
}
trap cleanup EXIT

# Удаляем старые логи
rm -f "$PGW_LOG" "$CDR_LOG" "$CLIENT_LOG"

echo ">>> Запускаем сервер:"
echo "    $PGW_SERVER $SERVER_CFG"
"$PGW_SERVER" "$SERVER_CFG" &
SERVER_PID=$!
sleep 1

echo
echo ">>> Клиент: создаём сессию для $NORMAL_IMSI"
"$PGW_CLIENT" "$CLIENT_CFG" "$NORMAL_IMSI"
echo

echo ">>> Клиент: пытаемся чёрный IMSI $BLACK_IMSI"
"$PGW_CLIENT" "$CLIENT_CFG" "$BLACK_IMSI"
echo

echo ">>> Клиент: повторная попытка $NORMAL_IMSI"
"$PGW_CLIENT" "$CLIENT_CFG" "$NORMAL_IMSI"
echo

echo ">>> HTTP /check_subscriber?imsi=$NORMAL_IMSI"
curl -s "http://127.0.0.1:8080/check_subscriber?imsi=$NORMAL_IMSI"
echo

echo ">>> HTTP /check_subscriber?imsi=$BLACK_IMSI"
curl -s "http://127.0.0.1:8080/check_subscriber?imsi=$BLACK_IMSI"
echo

# Ждём конфигурированный таймаут + 1 сек (30 + 1)
echo ">>> Ждём 31 секунду для истечения сессии..."
sleep 31

echo ">>> HTTP /check_subscriber?imsi=$NORMAL_IMSI после таймаута"
curl -s "http://127.0.0.1:8080/check_subscriber?imsi=$NORMAL_IMSI"
echo

echo ">>> HTTP /stop"
curl -s "http://127.0.0.1:8080/stop"
echo

# Небольшая пауза для graceful shutdown
sleep 2

echo
echo "=== Содержимое $PGW_LOG ==="
cat "$PGW_LOG" || echo "(файл не найден)"

echo
echo "=== Содержимое $CDR_LOG ==="
cat "$CDR_LOG" || echo "(файл не найден)"

echo
echo "=== Содержимое $CLIENT_LOG ==="
cat "$CLIENT_LOG" || echo "(файл не найден)"
