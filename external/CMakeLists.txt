add_library(tomlplusplus INTERFACE)
add_library(doctest INTERFACE)

target_include_directories(tomlplusplus 
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tomlplusplus)
target_include_directories(doctest
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/doctest)