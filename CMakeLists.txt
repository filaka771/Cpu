cmake_minimum_required(VERSION 3.10)

project(Cpu)

set(CMAKE_CXX_STANDARD 17)

# Find OpenSSL for SHA256 functions
find_package(OpenSSL REQUIRED)

add_library(instructions
  src/instructions/instructions.cpp
  src/cpu_emulator/cpu_instructions.cpp
  src/stack/stack.cpp
)

target_link_libraries(instructions PRIVATE OpenSSL::SSL OpenSSL::Crypto)


# Disassembler module build configuration
add_executable(disassembler src/disassembler/disassembler.cpp)
target_link_libraries(disassembler instructions)
target_include_directories(disassembler PRIVATE src/disassembler)

# Text ASM parser library
#add_library(text_asm_parser
#  src/assembler/text_asm_parser/text_asm_parser.cpp
#)
#target_include_directories(text_asm_parser
#  PUBLIC 
#  src/assembler/text_asm_parser
#)

# Assembler module build configuration
#add_subdirectory(text_asm_parser)

#add_executable(assembler src/assembler/assembler.cpp)
#target_link_libraries(assembler instructions text_asm_parser)
#target_include_directories(assembler PRIVATE src/assembler)


# Cpu emulator module build configuration
#add_executable(cpu_emulator /src/cpu_emulator/cpu.cpp)
#target_link_library(cpu_emulator instructions)
#target_include_directories(cpu_emulator PRIVATE src/cpu_emulator)


