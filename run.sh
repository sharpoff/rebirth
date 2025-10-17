#!/bin/bash
cmake --build build/ -j$(nproc) && ./build/src/main
