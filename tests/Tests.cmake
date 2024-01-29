find_package(CMOCKA REQUIRED)
find_program(MEMORYCHECK_COMMAND NAMES valgrind)

set(MEMORYCHECK_COMMAND_OPTIONS
    "--error-exitcode=1     \
    --leak-check=full       \
    --show-leak-kinds=all   \
    --leak-resolution=high  \
    --track-origins=yes     \
    --read-var-info=yes     \
    --child-silent-after-fork=yes"
)

include (CTest)

# parser test
add_executable(
    test_parser
    tests/test_parser.c
)

target_link_libraries(
    test_parser
    ${CMOCKA_LIBRARIES}
    uci2
)

set_target_properties(
    test_parser
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

add_test(
    NAME test_parser
    COMMAND test_parser
)

set_tests_properties(
    test_parser
    PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    ENVIRONMENT ASAN_OPTIONS=${ASAN_OPTIONS}
)

file(GLOB CONFIGURATION_FILES tests/files/*)
file(COPY ${CONFIGURATION_FILES} DESTINATION ${CMAKE_BINARY_DIR}/tests)
