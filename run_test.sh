#!/bin/bash
set -euo pipefail

if [[ $EUID -ne 0 ]]; then
    echo "This script must be run with sudo"
    exit 1
fi

INTERFACE="tun0"
IP_ADDR="10.0.0.2/24"
ROUTER_BIN="./build/LiteRouter"

if [[ ! -f "$ROUTER_BIN" ]]; then
    echo "[ERROR] No binary file. Run setup.sh first."
    exit 1
fi

cleanup() {
    if [[ -n "${ROUTER_PID:-}" ]] && kill -0 "$ROUTER_PID" 2>/dev/null; then
        kill "$ROUTER_PID" 2>/dev/null || true
        wait "$ROUTER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT

echo "Starting ICMP responder on TUN..."
$ROUTER_BIN &
ROUTER_PID=$!

echo "Waiting for interface ${INTERFACE}..."
for _ in $(seq 1 30); do
    if ip link show "$INTERFACE" &>/dev/null; then
        break
    fi
    sleep 0.2
done

if ! ip link show "$INTERFACE" &>/dev/null; then
    echo "[ERROR] Interface ${INTERFACE} did not appear in time"
    exit 1
fi

echo "Configuring ${INTERFACE}..."
ip link set dev "$INTERFACE" up
ip addr add "$IP_ADDR" dev "$INTERFACE" 2>/dev/null || true

echo "Testing ping..."
ping -c 3 -W 2 10.0.0.1
PING_STATUS=$?

if [[ $PING_STATUS -ne 0 ]]; then
    echo "[ERROR] ping failed with exit code ${PING_STATUS}"
    exit "$PING_STATUS"
fi

echo "Integration test finished successfully."
