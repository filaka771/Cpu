cmake_minimum_required(VERSION 3.10)
project(CPU)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

include_directories(
  cpu_backend/include
  vendor/include
)

add_subdirectory(vendor)
add_subdirectory(cpu_backend)
