cmake_minimum_required(VERSION 3.5)
project(CPU_Backend)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    cpu_backend/include
    vendor/include
)

add_subdirectory(vendor)
add_subdirectory(cpu_backend)
