add_library(tomlplusplus INTERFACE)
add_library(doctest INTERFACE)
add_library(stb_image STATIC stb_image/stb_image_impl.cpp)

target_include_directories(tomlplusplus 
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tomlplusplus)
target_include_directories(doctest
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/doctest)

target_include_directories(stb_image SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb_image)
if (MSVC)
  target_compile_options(stb_image PRIVATE "/W0")
else()
  target_compile_options(stb_image PRIVATE "-w")
endif()