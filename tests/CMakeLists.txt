include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(INSTALL_GTEST          OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(literouter_unit_tests
    unit/test_utils.cpp
    unit/test_protocol_core.cpp
    unit/test_routing_table.cpp
)

target_link_libraries(literouter_unit_tests
    PRIVATE
        LiteRouterLib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(literouter_unit_tests)
