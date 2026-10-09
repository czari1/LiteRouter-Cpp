# LiteRouter-Cpp

Linux userspace program that answers **ICMP Echo (ping)** on a **TUN** interface. It parses IPv4 headers, validates checksums, and sends Echo Reply — not a multi-interface IPv4 router (no routing table or packet forwarding yet).

## Features

- TUN device setup via `ioctl`
- IPv4 header parsing (`packed`, network byte order)
- Incoming IP and ICMP checksum verification
- ICMP Echo Request → Echo Reply
- Graceful shutdown on SIGINT/SIGTERM (descriptor closed in main loop)
- Host unit tests (no root, no `/dev/net/tun`)

## Requirements

- Linux
- GCC 10+ or Clang 10+ with C++20
- CMake 3.10+
- Root for running on TUN (not for unit tests)

## Build

```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

Or `./setup.sh`.

## Run

```bash
sudo ./build/LiteRouter
```

In another terminal:

```bash
sudo ip link set dev tun0 up
sudo ip addr add 10.0.0.1/24 dev tun0
ping 10.0.0.1
```

Integration script (checks `ping` exit code):

```bash
sudo ./run_test.sh
```

## Layout

- `include/ProtocolDefinition.hpp` — wire-format IPv4/ICMP headers
- `include/protocol_core.h`, `src/protocol_core.c` — C checksum/header helpers (host-tested, MCU-friendly)
- `src/Router.cpp` — packet handling
- `tests/` — GoogleTest unit tests

## License

MIT — see [LICENSE](LICENSE).
