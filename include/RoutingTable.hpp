#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <arpa/inet.h>

namespace Net {
    
    struct __attribute__((packed)) RouteEntry {
        uint32_t destination;
        uint32_t netmask;
        uint32_t next_hop;
        std::string interface;
        uint8_t prefix_length;
        uint32_t metric;
    
    
        bool matchesIPdest(uint32_t destIP) const {
            return (destIP & netmask) == (destination & netmask);
        }

        bool isDirectlyConnected() const {
            return next_hop == 0;
        }

        std::string toString() const;
    };

    class RoutingTable {
    public:
        RoutingTable() = default;

        bool addRoute(uint32_t destination, uint8_t prefix_length, 
                    uint32_t next_hop, const std::string interface,
                    uint32_t metric = 0);

        bool addRoute (const std::string& destination, uint8_t prefix_length, 
                    const std::string& next_hop, const std::string& interface,
                    uint32_t metric = 0);

        bool removeRoute(uint32_t destination, uint8_t prefix_length);
        void removeRoutesForInterface(const std::string& interface_name);
        std::optional<RouteEntry> lookup(uint32_t destIP) const;
        const std::vector<RouteEntry>& getRoutes() const;
        bool loadFromFile(const std::string& filename);
        bool saveToFile(const std::string& filename) const;
        void clearRoutes();
        size_t getTableSize() const;
    private:
        std::vector<RouteEntry> m_routes;
        void sortRoutes();
        static uint32_t prefixToNetmask(uint8_t prefix_length);
    };

    uint32_t ipStringToUint_32(const std::string& ipStr);
    std::string uint32_tToString(uint32_t ipAddr);
    bool parseCIDR(const std::string& cidr, uint32_t& outIP, uint8_t& outPrefix);
}