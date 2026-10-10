#include <gtest/gtest.h>

#include "Exceptions.hpp"
#include "RoutingTable.hpp"

#include <arpa/inet.h>
#include <cstdio>
#include <string>

namespace {
    const char* kRouteFile = "literouter_routes_test.txt";
}

TEST(RoutingTable, LongestPrefixThenMetric) {
    Net::RoutingTable table;
    ASSERT_TRUE(table.addRoute(htonl(0x0A000000), 8, htonl(0x01010101), "eth0", 20));
    ASSERT_TRUE(table.addRoute(htonl(0x0A000000), 8, htonl(0x02020202), "eth1", 5));
    ASSERT_TRUE(table.addRoute(htonl(0x0A010000), 16, 0, "tun0", 100));
    ASSERT_TRUE(table.addRoute(0, 0, htonl(0x0A000001), "tun0", 200));

    const auto host = table.lookup(htonl(0x0A010203));
    ASSERT_TRUE(host.has_value());
    EXPECT_EQ(host->prefix_length, 16);
    EXPECT_TRUE(host->isDirectlyConnected());
    EXPECT_EQ(host->interface, "tun0");

    const auto net = table.lookup(htonl(0x0A020304));
    ASSERT_TRUE(net.has_value());
    EXPECT_EQ(net->prefix_length, 8);
    EXPECT_EQ(net->next_hop, htonl(0x02020202));
    EXPECT_EQ(net->interface, "eth1");

    const auto fallback = table.lookup(htonl(0x08080808));
    ASSERT_TRUE(fallback.has_value());
    EXPECT_EQ(fallback->prefix_length, 0);
}

TEST(RoutingTable, RejectsInvalidRoutesAndDuplicates) {
    Net::RoutingTable table;
    EXPECT_FALSE(table.addRoute(htonl(0x0A000000), 33, 0, "tun0"));
    EXPECT_FALSE(table.addRoute(htonl(0x0A000000), 24, 0, ""));
    EXPECT_FALSE(table.addRoute("10.0.0", 24, "-", "tun0"));
    ASSERT_TRUE(table.addRoute("10.0.0.5", 24, "-", "tun0", 1));
    EXPECT_FALSE(table.addRoute("10.0.0.9", 24, "direct", "tun0", 1));
    EXPECT_EQ(table.getTableSize(), 1U);
    EXPECT_EQ(table.getRoutes().front().destination, htonl(0x0A000000));
}

TEST(RoutingTable, RemoveAndClear) {
    Net::RoutingTable table;
    ASSERT_TRUE(table.addRoute("10.0.0.0", 8, "10.0.0.1", "eth0"));
    ASSERT_TRUE(table.addRoute("192.168.0.0", 16, "-", "wlan0"));
    table.removeRoutesForInterface("wlan0");
    EXPECT_EQ(table.getTableSize(), 1U);
    ASSERT_TRUE(table.removeRoute(htonl(0x0A000000), 8));
    EXPECT_EQ(table.getTableSize(), 0U);
    EXPECT_FALSE(table.lookup(htonl(0x0A010203)).has_value());
    table.clearRoutes();
    EXPECT_EQ(table.getTableSize(), 0U);
}

TEST(RoutingTable, FileRoundTripKeepsExistingTableOnFailure) {
    Net::RoutingTable table;
    ASSERT_TRUE(table.addRoute("10.1.0.0", 16, "10.0.0.1", "eth0", 7));
    ASSERT_TRUE(table.addRoute("0.0.0.0", 0, "-", "tun0", 100));
    std::remove(kRouteFile);
    ASSERT_TRUE(table.saveToFile(kRouteFile));

    Net::RoutingTable loaded;
    ASSERT_TRUE(loaded.addRoute("1.2.3.0", 24, "-", "keep"));
    ASSERT_FALSE(loaded.loadFromFile("literouter_missing_routes.txt"));
    EXPECT_EQ(loaded.getTableSize(), 1U);

    ASSERT_TRUE(loaded.loadFromFile(kRouteFile));
    EXPECT_EQ(loaded.getTableSize(), 2U);
    const auto route = loaded.lookup(htonl(0x0A010203));
    ASSERT_TRUE(route.has_value());
    EXPECT_EQ(route->metric, 7U);
    EXPECT_EQ(route->next_hop, htonl(0x0A000001));
    EXPECT_NE(route->toString().find("10.1.0.0/16"), std::string::npos);

    std::remove(kRouteFile);
}

TEST(RoutingTable, AddressHelpers) {
    uint32_t ip = 0;
    uint8_t prefix = 0;
    ASSERT_TRUE(Net::parseCIDR("10.0.0.0/24", ip, prefix));
    EXPECT_EQ(prefix, 24);
    EXPECT_EQ(ip, htonl(0x0A000000));
    EXPECT_FALSE(Net::parseCIDR("10.0.0.0/33", ip, prefix));
    EXPECT_FALSE(Net::parseCIDR("10.0.0.0", ip, prefix));
    EXPECT_EQ(Net::ipStringToUint_32("10.0.0.1"), htonl(0x0A000001));
    EXPECT_EQ(Net::uint32_tToString(htonl(0x0A000001)), "10.0.0.1");
    EXPECT_THROW(Net::ipStringToUint_32("nope"), Exceptions::InvalidConfigException);
}
