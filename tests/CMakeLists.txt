add_executable(re_tests
  main.cpp
  unit/test_sizeof_layout.cpp
  unit/test_aabb.cpp
  unit/test_transform.cpp
)

target_link_libraries(re_tests PRIVATE doctest re_core)

include(CTest)
add_test(NAME unit COMMAND re_tests)