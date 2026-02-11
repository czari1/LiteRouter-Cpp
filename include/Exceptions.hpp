#pragma once

#include <stdexcept>
#include <string>
#include <cstdint>

namespace Exceptions {
    
    class RouterException : public std::runtime_error {
    public:
        explicit RouterException(const std::string& message)
            : std::runtime_error(message) {}
    };

    class TunDeviceException : public RouterException {
    public:
        explicit TunDeviceException(const std::string& message)
            : RouterException("TUN DEvice Error: " + message) {}
    };

    class TunDeviceOpenException : public TunDeviceException {
    public:
        explicit TunDeviceOpenException(const std::string& device)
            : TunDeviceException("Failed to open device '" + device + "'. Root permission required?") {}      
    };

    class TunDeviceConfigException : public TunDeviceException {
    public:
        explicit TunDeviceConfigException(const std::string& details)
            : TunDeviceException("Configuration failed: " + details) {}
    };

    class TunDeviceReadException : public TunDeviceException {
    public: 
        explicit TunDeviceReadException(const std::string& details)
            : TunDeviceException("Read operation failed: " + details) {}
    };

    class TunDeviceWriteException : public TunDeviceException {
    public: 
        explicit TunDeviceWriteException(const std::string& details)
            : TunDeviceException("Write operation failed: " + details) {}
    };

    class PacketException : public RouterException {
    public:
        explicit PacketException(const std::string& message) 
            : RouterException("Packet error: " + message) {}
    };

    class InvalidPacketException : public PacketException {
    public:
        explicit InvalidPacketException(const std::string& reason)
            : PacketException("Invalid packet: " + reason) {}
    };

    class PacketTooShortException : public InvalidPacketException {
    public:
        explicit PacketTooShortException(size_t received, size_t required)
            : InvalidPacketException("Packet too short. Received: " + std::to_string(received) 
                                    +   " bytes. Required: " + std::to_string(required) + "bytes.") {}
    };

    class UnsupportedProtocolException : public PacketException {
    public:
        explicit UnsupportedProtocolException(uint8_t protocol)
            : PacketException("Unsupported protocol: " + std::to_string(protocol)) {}
    };

    class ChecksumMissmatchException : public PacketException {
    public:
        ChecksumMissmatchException(uint16_t expected, uint16_t actual)
            : PacketException("Checksum missmatch. Expected: " + std::to_string(expected) + 
            ", Actual: " + std::to_string(actual)) {} 
    };

    class ConfigException : public RouterException {
    public:
        explicit ConfigException(const std::string& message)
            : RouterException("Configuration error: " + message) {}
    };

    class InvalidConfigException : public ConfigException {
    public: 
        explicit InvalidConfigException(const std::string& parameter)
            : ConfigException("Invalid confuguration parameter: " + parameter) {}
    };
}