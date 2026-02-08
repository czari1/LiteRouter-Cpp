cmake_minimum_required(VERSION 3.10)

project(LiteRouter-Cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

add_compile_options(-Wall -Wextra -pedantic -O2)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/TunDevice.cpp
    src/Router.cpp
    src/Utils.cpp
)

add_executable(LiteRouter ${SOURCES})