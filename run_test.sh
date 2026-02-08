#!/bin/bash

if [[ $EUID -ne 0 ]]; then
    echo "This script have to be started with sudo permission"
    exit 1
fi

INTERFACE="tun0"
IP_ADDR="10.0.0.2/24"
ROUTER_BIN="./build/LiteRouter"

if [[ ! -f "$ROUTER_BIN" ]]; then
    echo "[ERROR] No binary file. Run setup.sh first."
    exit 1
fi

echo "Router setup"
$ROUTER_BIN &
ROUTER_PID=$!

sleep 1

echo "Configuration of interface $INTERFACE..."
ip link set dev $INTERFACE up
ip addr add $IP_ADDR dev $INTERFACE

echo "Testing PING"
ping -c 3 10.0.0.1

echo "Clean up"
kill $ROUTER_PID
echo "Test finished."
