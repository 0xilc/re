add_executable(re_tests
)
target_link_libraries(re_tests PRIVATE re_core)

include(CTest)
add_test(NAME unit COMMAND re_tests)