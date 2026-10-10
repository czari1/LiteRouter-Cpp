cmake_minimum_required(VERSION 3.10)

project(LiteRouter-Cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -O2)

include_directories(include)

find_package(Threads REQUIRED)

set(LITEROUTER_LIB_SOURCES
    src/TunDevice.cpp
    src/Router.cpp
    src/Utils.cpp
    src/Logger.cpp
    src/RoutingTable.cpp
    src/protocol_core.cpp
)

add_library(LiteRouterLib STATIC ${LITEROUTER_LIB_SOURCES})
target_include_directories(LiteRouterLib PUBLIC include)
target_link_libraries(LiteRouterLib PUBLIC Threads::Threads)

add_executable(LiteRouter src/main.cpp)
target_link_libraries(LiteRouter PRIVATE LiteRouterLib)

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
