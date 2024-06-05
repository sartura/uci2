cmake_minimum_required(VERSION 3.8)
project(uci2 C)

# compile options
include(CompileOptions.cmake)

# det module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# include dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

# uci2 library
set(
    SOURCE_FILES
    src/uci2.c
    src/ast.c
    src/lexer.c
    src/parser.c
    src/utils/memory.c
)

add_library(
    uci2
    SHARED
    ${SOURCE_FILES}
)

add_library(
    uci2_static
    STATIC
    ${SOURCE_FILES}
)

set_target_properties(
    uci2 uci2_static
    PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
)

set_target_properties(
    uci2 uci2_static
    PROPERTIES
    PUBLIC_HEADER "src/uci2.h"
)

# install uci2
install(
    TARGETS
    uci2
    LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER
    DESTINATION include
)

install(
    TARGETS
    uci2_static
    ARCHIVE
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER
    DESTINATION include
)

# tests
if(ENABLE_TESTS)
    include(tests/Tests.cmake)
endif()