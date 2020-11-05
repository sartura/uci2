# parser test
add_executable(test_parser test_parser.c)
add_test(NAME test_parser COMMAND test_parser ${CMAKE_SOURCE_DIR}/tests/cfg/)
target_link_libraries(test_parser uci2 ${CMOCKA_LIBRARIES})
