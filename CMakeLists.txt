cmake_minimum_required(VERSION 3.10)

project(LiteRouter-Cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -O2)

include_directories(include)

set(LITEROUTER_LIB_SOURCES
    src/TunDevice.cpp
    src/Router.cpp
    src/Utils.cpp
    src/Logger.cpp
    src/protocol_core.c
)

add_library(LiteRouterLib STATIC ${LITEROUTER_LIB_SOURCES})
target_include_directories(LiteRouterLib PUBLIC include)

add_executable(LiteRouter src/main.cpp)
target_link_libraries(LiteRouter PRIVATE LiteRouterLib)

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
