cmake_minimum_required(VERSION 2.8)
project(uci2 C)

# compile options
include(CompileOptions.cmake)

# include dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

# libuci2 library
add_library(${PROJECT_NAME} SHARED
        src/libuci2.c
        src/uci2_ast.c
        src/uci2_lexer.c
        src/uci2_parser.c
)

# header files
set(UCI2_HEADERS
    "src/libuci2.h"
    "src/uci2_ast.h"
)

set_target_properties(${PROJECT_NAME}
                      PROPERTIES
                        PUBLIC_HEADER "${UCI2_HEADERS}"
)

# example programs
add_executable(example_check src/examples/example_check.c)
add_executable(example_new src/examples/example_new.c)


# det module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# link examples
target_link_libraries(example_check ${PROJECT_NAME})
target_link_libraries(example_new ${PROJECT_NAME})

# install libuci2
install(TARGETS ${PROJECT_NAME} 
        LIBRARY 
	    DESTINATION lib ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER
            DESTINATION include
)

# install examples
install(TARGETS example_check DESTINATION bin)
install(TARGETS example_new DESTINATION bin)

# tests
if(ENABLE_TESTS)
    find_package(CMOCKA REQUIRED)
    enable_testing()
    add_subdirectory(tests)
endif()

INCLUDE(Dart)
find_program(MEMORYCHECK_COMMAND NAMES valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS "--error-exitcode=1 --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --read-var-info=yes  --child-silent-after-fork=yes")
