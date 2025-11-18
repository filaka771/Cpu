cmake_minimum_required(VERSION 3.10)

project(Cpu)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(assembler)
add_subdirectory(cpu_emulator)
add_subdirectory(disassembler)
add_subdirectory(errors)
add_subdirectory(instructions)
add_subdirectory(text_asm_parser)
add_subdirectory(vendor)

