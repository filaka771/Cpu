# Stack library
add_library(stack_lib STATIC
    src/stack/stack.cpp
)

target_include_directories(stack_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
