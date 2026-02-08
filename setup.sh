#!/bin/bash
set -e

echo "Project setup"

mkdir -p build
cd build 
cmake ..
make

echo "Setup finished"
echo "Start it by writing sudo ./build/LiteRouter"