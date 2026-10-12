cmake_minimum_required(VERSION 3.20)
project(pi_tracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(hello src/main.cpp)