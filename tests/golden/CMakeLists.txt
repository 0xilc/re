re_add_test_binary(re_tests_golden golden slow
  test_golden.cpp
)

target_link_libraries(re_tests_golden PRIVATE stb_image)
set_tests_properties(golden PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})