add_library(re_test_main OBJECT main.cpp)
target_link_libraries(re_test_main PUBLIC doctest)

function(re_add_test_binary target name label)
  add_executable(${target} ${ARGN} $<TARGET_OBJECTS:re_test_main>)
  target_link_libraries(${target} PRIVATE doctest re_core)
  target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/tests)

  add_test(NAME ${name} COMMAND ${target})
  set_tests_properties(${name} PROPERTIES LABELS "${label}" TIMEOUT 300)

  add_custom_target(test-${name}
    COMMAND $<TARGET_FILE:${target}>
    DEPENDS ${target}
    COMMENT "Running ${name} tests"
    USES_TERMINAL
  )
endfunction()

add_subdirectory(unit)
add_subdirectory(bxdf)
add_subdirectory(integration)

add_custom_target(test-all COMMENT "Running all tests")
add_dependencies(test-all test-unit test-bxdf test-integration)