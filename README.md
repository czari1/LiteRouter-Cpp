# LiteRouter-Cpp

Linux userspace program that answers **ICMP Echo (ping)** on a **TUN** interface. It parses IPv4 headers, validates checksums, and sends Echo Reply. A longest-prefix routing table is included and covered by unit tests. The TUN loop answers echo requests.

## Features

- TUN device setup via `ioctl`
- IPv4 header parsing (`packed`, network byte order)
- Incoming IP and ICMP checksum verification
- ICMP Echo Request → Echo Reply
- Longest-prefix routing table (lookup, metrics, load/save)
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
sudo ip addr add 10.0.0.2/24 dev tun0
ping -c 3 10.0.0.1
```

`10.0.0.2/24` is the address on the host. `10.0.0.1` is answered by LiteRouter. Assigning `10.0.0.1` to `tun0` and pinging that same address is handled by the kernel and never reaches the program.

Integration script (checks `ping` exit code):

```bash
sudo ./run_test.sh
```

## Layout

- `include/ProtocolDefinition.hpp` — wire-format IPv4/ICMP headers
- `include/protocol_core.hpp`, `src/protocol_core.cpp` — checksum and IPv4 header helpers
- `include/RoutingTable.hpp` — longest-prefix routing table
- `src/Router.cpp` — packet handling
- `tests/` — GoogleTest unit tests

