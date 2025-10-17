cmake_minimum_required(VERSION 3.50)
project(Rebirth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(external)
add_subdirectory(src)