add_executable(re_tests
  main.cpp
  unit/test_sizeof_layout.cpp
  unit/test_aabb.cpp
  unit/test_transform.cpp
  unit/test_triangle_intersection.cpp
  unit/test_triangle.cpp
  unit/test_sampler.cpp
  bxdf/test_chi_square.cpp
  bxdf/test_white_furnace.cpp
  bxdf/test_reciprocity.cpp
  bxdf/test_sampling_consistency.cpp
)

target_link_libraries(re_tests PRIVATE doctest re_core)

include(CTest)
add_test(NAME unit COMMAND re_tests)