cmake_minimum_required(VERSION 3.8)
project(uci2 C)

# compile options
include(CompileOptions.cmake)

# det module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# include dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

# libuci2 library
add_library(
    ${PROJECT_NAME}
    SHARED
    src/libuci2.c
    src/uci2_ast.c
    src/uci2_lexer.c
    src/uci2_parser.c
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    PUBLIC_HEADER
    "src/uci2_ast.h;src/libuci2.h"
)

# example programs
add_executable(
    example_check
    src/examples/example_check.c
)

add_executable(
    example_new
    src/examples/example_new.c
)

# link examples
target_link_libraries(
    example_check
    ${PROJECT_NAME}
)

target_link_libraries(
    example_new
    ${PROJECT_NAME}
)

# install libuci2
install(
    TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION lib ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER
    DESTINATION include
)

# tests
if(ENABLE_TESTS)
    include(tests/Tests.cmake)
endif()
