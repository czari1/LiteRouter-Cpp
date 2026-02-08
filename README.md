# LiteRouter-Cpp

A lightweight IPv4 router implementation in C++20 that uses TUN/TAP devices to handle network packets at the IP layer. Currently supports ICMP echo (ping) requests and replies.

## Features

- **TUN Device Management**: Creates and manages virtual network interfaces
- **IPv4 Packet Handling**: Parses and processes IPv4 headers
- **ICMP Protocol Support**: Responds to ping requests (ICMP Echo Request/Reply)
- **Checksum Calculation**: Proper IPv4 and ICMP checksum computation
- **Clean Architecture**: Separation of concerns with dedicated classes for routing, device management, and utilities

## Requirements

- **Operating System**: Linux (uses Linux TUN/TAP interface)
- **Compiler**: C++20 compatible compiler (GCC 10+ or Clang 10+)
- **CMake**: Version 3.10 or higher
- **Root Privileges**: Required for TUN device creation and network operations

## Project Structure

```
LiteRouter-Cpp/
├── include/
│   ├── ProtocolDefinition.hpp  # Network protocol headers (IPv4, ICMP)
│   ├── Router.hpp               # Router class declaration
│   ├── TunDevice.hpp            # TUN device wrapper
│   └── Utils.hpp                # Utility functions (checksum)
├── src/
│   ├── main.cpp                 # Application entry point
│   ├── Router.cpp               # Packet routing logic
│   ├── TunDevice.cpp            # TUN device implementation
│   └── Utils.cpp                # Utility implementations
├── CMakeLists.txt               # Build configuration
├── setup.sh                     # Build script
└── run_test.sh                  # Automated testing script
```

## Building the Project

### Quick Setup

Run the setup script to build the project:

```bash
./setup.sh
```

This will:
- Create a `build/` directory
- Run CMake configuration
- Compile the project

### Manual Build

Alternatively, build manually:

```bash
mkdir -p build
cd build
cmake ..
make
```

## Running the Router

The router requires root privileges to create TUN devices:

```bash
sudo ./build/LiteRouter
```

After starting, the router will display instructions. In a second terminal, configure the TUN interface:

```bash
sudo ip link set dev tun0 up
sudo ip addr add 10.0.0.1/24 dev tun0
```

Now you can test the router by pinging it:

```bash
ping 10.0.0.1
```

## Automated Testing

Use the provided test script to automatically set up and test the router:

```bash
sudo ./run_test.sh
```

This script will:
1. Start the router in the background
2. Configure the `tun0` interface with IP `10.0.0.2/24`
3. Send 3 ping packets to `10.0.0.1`
4. Clean up and terminate the router

## How It Works

### TUN Device

The router creates a virtual network interface (`tun0`) that operates at the IP layer. All packets sent to this interface are read by the router application for processing.

### Packet Flow

1. Application reads raw IP packets from the TUN device
2. Router parses the IPv4 header
3. Based on protocol type, router dispatches to appropriate handler
4. For ICMP Echo Requests (ping), router:
   - Swaps source and destination IP addresses
   - Changes ICMP type from 8 (request) to 0 (reply)
   - Recalculates checksums
   - Writes the reply back to the TUN device

### Supported Protocols

- **ICMP (Protocol 1)**: Echo Request/Reply (ping)
- Other protocols are logged but not yet implemented

## Stopping the Router

Press `Ctrl+C` to gracefully shut down the router. The signal handler will clean up resources and close the TUN device.

## Troubleshooting

### "Opening error /dev/net/tun"
- Ensure you're running with root privileges (`sudo`)
- Verify TUN/TAP kernel module is loaded: `lsmod | grep tun`

### "Cannot open an interface"
- The interface name might already be in use
- Try a different interface name in the code

### No ping response
- Verify the TUN interface is up: `ip link show tun0`
- Check interface has correct IP: `ip addr show tun0`
- Ensure no firewall rules are blocking ICMP

## License

This project is provided as-is for educational purposes.
