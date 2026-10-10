#include "RoutingTable.hpp"
#include "Exceptions.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>

#include <arpa/inet.h>

namespace Net {
    namespace {
        bool parseIPv4(const std::string& ipStr, uint32_t& out) {
            in_addr addr{};
            if (inet_pton(AF_INET, ipStr.c_str(), &addr) != 1) {
                return false;
            }
            out = addr.s_addr;
            return true;
        }

        bool parseUint32(const std::string& text, uint32_t& out) {
            if (text.empty()) {
                return false;
            }
            uint64_t value = 0;
            for (char c : text) {
                if (c < '0' || c > '9') {
                    return false;
                }
                value = value * 10ULL + static_cast<uint64_t>(c - '0');
                if (value > 0xFFFFFFFFULL) {
                    return false;
                }
            }
            out = static_cast<uint32_t>(value);
            return true;
        }

        bool parsePrefix(const std::string& text, uint8_t& out) {
            uint32_t value = 0;
            if (!parseUint32(text, value) || value > 32U) {
                return false;
            }
            out = static_cast<uint8_t>(value);
            return true;
        }

        bool parseNextHop(const std::string& text, uint32_t& out) {
            if (text == "-" || text == "direct" || text == "0.0.0.0") {
                out = 0;
                return true;
            }
            return parseIPv4(text, out);
        }

        std::string trimCopy(const std::string& text) {
            const auto begin = text.find_first_not_of(" \t\r");
            if (begin == std::string::npos) {
                return {};
            }
            const auto end = text.find_last_not_of(" \t\r");
            return text.substr(begin, end - begin + 1);
        }
    }

    uint32_t RoutingTable::prefixToNetmask(uint8_t prefix_length) {
        if (prefix_length == 0 || prefix_length > 32) {
            return 0;
        }
        const uint32_t hostMask = 0xFFFFFFFFu << (32U - prefix_length);
        return htonl(hostMask);
    }

    void RoutingTable::sortRoutes() {
        std::stable_sort(m_routes.begin(), m_routes.end(),
                         [](const RouteEntry& a, const RouteEntry& b) {
                             if (a.prefix_length != b.prefix_length) {
                                 return a.prefix_length > b.prefix_length;
                             }
                             return a.metric < b.metric;
                         });
    }

    bool RoutingTable::addRoute(uint32_t destination, uint8_t prefix_length,
                                uint32_t next_hop, const std::string& interfaceName,
                                uint32_t metric) {
        if (prefix_length > 32 || interfaceName.empty()) {
            return false;
        }

        const uint32_t netmask = prefixToNetmask(prefix_length);
        destination &= netmask;

        for (const RouteEntry& route : m_routes) {
            if (route.destination == destination && route.prefix_length == prefix_length &&
                route.next_hop == next_hop && route.interface == interfaceName) {
                return false;
            }
        }

        RouteEntry entry;
        entry.destination = destination;
        entry.netmask = netmask;
        entry.next_hop = next_hop;
        entry.interface = interfaceName;
        entry.prefix_length = prefix_length;
        entry.metric = metric;
        m_routes.push_back(std::move(entry));
        sortRoutes();
        return true;
    }

    bool RoutingTable::addRoute(const std::string& destination, uint8_t prefix_length,
                                const std::string& next_hop, const std::string& interfaceName,
                                uint32_t metric) {
        uint32_t dest = 0;
        uint32_t hop = 0;
        if (!parseIPv4(destination, dest) || !parseNextHop(next_hop, hop)) {
            return false;
        }
        return addRoute(dest, prefix_length, hop, interfaceName, metric);
    }

    bool RoutingTable::removeRoute(uint32_t destination, uint8_t prefix_length) {
        if (prefix_length > 32) {
            return false;
        }
        destination &= prefixToNetmask(prefix_length);
        const auto removed = std::erase_if(m_routes, [&](const RouteEntry& route) {
            return route.destination == destination && route.prefix_length == prefix_length;
        });
        return removed > 0;
    }

    void RoutingTable::removeRoutesForInterface(const std::string& interface_name) {
        std::erase_if(m_routes, [&](const RouteEntry& route) {
            return route.interface == interface_name;
        });
    }

    std::optional<RouteEntry> RoutingTable::lookup(uint32_t destIP) const {
        for (const RouteEntry& route : m_routes) {
            if (route.matchesIPdest(destIP)) {
                return route;
            }
        }
        return std::nullopt;
    }

    const std::vector<RouteEntry>& RoutingTable::getRoutes() const {
        return m_routes;
    }

    bool RoutingTable::loadFromFile(const std::string& filename) {
        std::ifstream input(filename);
        if (!input) {
            return false;
        }

        RoutingTable loaded;
        std::string line;
        while (std::getline(input, line)) {
            line = trimCopy(line);
            if (line.empty() || line[0] == '#') {
                continue;
            }

            std::istringstream iss(line);
            std::string cidr;
            std::string nextHop;
            std::string interfaceName;
            if (!(iss >> cidr >> nextHop >> interfaceName)) {
                return false;
            }

            uint32_t metric = 0;
            std::string metricText;
            if (iss >> metricText) {
                if (!parseUint32(metricText, metric)) {
                    return false;
                }
            }
            std::string extra;
            if (iss >> extra) {
                return false;
            }

            uint32_t destination = 0;
            uint8_t prefix = 0;
            uint32_t hop = 0;
            if (!parseCIDR(cidr, destination, prefix) || !parseNextHop(nextHop, hop)) {
                return false;
            }
            if (!loaded.addRoute(destination, prefix, hop, interfaceName, metric)) {
                return false;
            }
        }

        m_routes = std::move(loaded.m_routes);
        return true;
    }

    bool RoutingTable::saveToFile(const std::string& filename) const {
        std::ofstream output(filename, std::ios::trunc);
        if (!output) {
            return false;
        }

        output << "# dest/prefix next_hop interface metric\n";
        for (const RouteEntry& route : m_routes) {
            output << uint32_tToString(route.destination) << "/"
                   << static_cast<int>(route.prefix_length) << " "
                   << (route.isDirectlyConnected() ? "-" : uint32_tToString(route.next_hop)) << " "
                   << route.interface << " " << route.metric << "\n";
        }
        return static_cast<bool>(output);
    }

    void RoutingTable::clearRoutes() {
        m_routes.clear();
    }

    size_t RoutingTable::getTableSize() const {
        return m_routes.size();
    }

    std::string RouteEntry::toString() const {
        std::ostringstream oss;
        oss << uint32_tToString(destination) << "/" << static_cast<int>(prefix_length)
            << " via " << (isDirectlyConnected() ? "direct" : uint32_tToString(next_hop))
            << " dev " << interface << " metric " << metric;
        return oss.str();
    }

    uint32_t ipStringToUint_32(const std::string& ipStr) {
        uint32_t address = 0;
        if (!parseIPv4(ipStr, address)) {
            throw Exceptions::InvalidConfigException(ipStr);
        }
        return address;
    }

    std::string uint32_tToString(uint32_t ipAddr) {
        char buffer[INET_ADDRSTRLEN];
        in_addr addr{};
        addr.s_addr = ipAddr;
        if (inet_ntop(AF_INET, &addr, buffer, INET_ADDRSTRLEN) == nullptr) {
            return "invalid";
        }
        return std::string(buffer);
    }

    bool parseCIDR(const std::string& cidr, uint32_t& outIP, uint8_t& outPrefix) {
        const auto slash = cidr.find('/');
        if (slash == std::string::npos || slash == 0 || slash + 1 >= cidr.size()) {
            return false;
        }
        if (!parseIPv4(cidr.substr(0, slash), outIP)) {
            return false;
        }
        return parsePrefix(cidr.substr(slash + 1), outPrefix);
    }
}
