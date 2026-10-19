cmake_minimum_required(VERSION 3.20)
project(pi_tracker LANGUAGES CXX)
find_package(Threads REQUIRED)                           

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)

add_library(pitracker
    src/fd.cpp
)

target_link_libraries(pitracker PUBLIC Threads::Threads)
target_include_directories(pitracker PUBLIC include)

add_executable(hello src/main.cpp)

file(GLOB SANDBOX_SOURCES CONFIGURE_DEPENDS sandbox/*.cpp)
foreach(src ${SANDBOX_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE pitracker)
endforeach()

target_sources(ch2_8_files PRIVATE sandbox/helper/ch2_8_get_integer.cpp)