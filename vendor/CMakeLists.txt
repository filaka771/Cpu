add_library(tools_lib STATIC
    src/stack/stack.cpp    # This is OUR file to manage
)

target_include_directories(tools_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
