#!/usr/bin/env bash
if [ ! -d "./build" ]; then
    mkdir ./build
fi

cd ./build || { echo "Failed to change directory to ./build"; exit 1; }
rm -rf *

if ! cmake ..; then
    echo "CMake failed"
    exit 1
fi

if ! make; then
    echo "Make failed"
    exit 1
fi

if ! sudo mv ./cpp_project /usr/bin/cpp_project; then
    echo "Failed to move cpp_project to /usr/bin. Ensure you have the necessary permissions."
    exit 1
fi