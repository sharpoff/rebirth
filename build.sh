#!/bin/bash
cmake -B build/debug -D CMAKE_BUILD_TYPE=Debug -D CMAKE_EXPORT_COMPILE_COMMANDS=1 -D CMAKE_POLICY_VERSION_MINIMUM=3.5 && cmake --build build/debug -j$(nproc)
#cmake -B build/release -D CMAKE_BUILD_TYPE=Release -D CMAKE_EXPORT_COMPILE_COMMANDS=1 -D CMAKE_POLICY_VERSION_MINIMUM=3.5 && cmake --build build/release -j$(nproc)
