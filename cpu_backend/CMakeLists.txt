# Main instructions library (as you requested)
add_library(instructions_lib STATIC
  src/instructions/instructions.cpp
  src/cpu_emulator/cpu.cpp
  src/cpu_emulator/cpu_instructions.cpp  
  vendor/src/stack/stack.cpp  # Or link against stack_lib
)

target_include_directories(instructions_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/include
)

target_link_libraries(instructions_lib stack_lib)

# Individual executables
add_executable(assembler src/assembler/assembler.cpp)
add_executable(disassembler src/disassembler/disassembler.cpp) 
add_executable(cpu_emulator src/cpu_emulator/cpu.cpp)  # Main CPU executable

# Link executables with the library
target_link_libraries(assembler instructions_lib)
target_link_libraries(disassembler instructions_lib)
target_link_libraries(cpu_emulator instructions_lib)
