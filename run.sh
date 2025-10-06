#!/bin/bash
cmake --build build/ -j$(nproc) && ./build/rebirth/main
