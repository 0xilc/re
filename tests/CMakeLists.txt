add_executable(re_tests
  main.cpp
  unit/test_sizeof_layout.cpp
)

target_link_libraries(re_tests PRIVATE doctest re_core)

include(CTest)
add_test(NAME unit COMMAND re_tests)