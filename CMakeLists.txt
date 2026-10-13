cmake_minimum_required(VERSION 3.20)
project(pi_tracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(hello src/main.cpp)

file(GLOB SANDBOX_SOURCES CONFIGURE_DEPENDS sandbox/*.cpp)
foreach(src ${SANDBOX_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
endforeach()

target_sources(ch2_8_files PRIVATE sandbox/helper/ch2_8_get_integer.cpp)