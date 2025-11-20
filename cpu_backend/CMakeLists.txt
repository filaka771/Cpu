find_package(OpenSSL REQUIRED)

add_library(main_product STATIC
    src/instructions/instructions.cpp
    src/cpu_emulator/cpu.cpp
    src/cpu_emulator/cpu_instructions.cpp
)

add_library(parser STATIC
    src/text_asm_parser/text_asm_parser.cpp
)

target_include_directories(main_product PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(main_product
    PRIVATE
    tools_lib
    OpenSSL::SSL
    OpenSSL::Crypto
)

add_executable(assembler src/assembler/assembler.cpp)
add_executable(disassembler src/disassembler/disassembler.cpp)
add_executable(cpu_emulator src/cpu_emulator/cpu.cpp)

target_link_libraries(assembler 
    PRIVATE 
    main_product 
    parser
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_link_libraries(disassembler 
    PRIVATE 
    main_product
) 

target_link_libraries(cpu_emulator 
    PRIVATE 
    main_product
    OpenSSL::SSL
    OpenSSL::Crypto
)
