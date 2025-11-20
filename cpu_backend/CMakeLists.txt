add_library(main_product STATIC
    src/instructions/instructions.cpp      # OUR code
    src/cpu_emulator/cpu.cpp              # OUR code  
    src/cpu_emulator/cpu_instructions.cpp # OUR code
)

target_include_directories(main_product PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(main_product tools_lib)

add_executable(assembler src/assembler/assembler.cpp)
add_executable(disassembler src/disassembler/disassembler.cpp)
add_executable(cpu_emulator src/cpu_emulator/cpu.cpp)

target_link_libraries(assembler main_product)
target_link_libraries(disassembler main_product) 
target_link_libraries(cpu_emulator main_product)
