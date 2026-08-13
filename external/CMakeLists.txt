# Third-party dependencies

add_library(tomlplusplus INTERFACE)
target_include_directories(tomlplusplus
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tomlplusplus)


add_library(doctest INTERFACE)
target_include_directories(doctest
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/doctest)


add_library(stb_image STATIC stb_image/stb_image_impl.cpp)
target_include_directories(stb_image SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb_image)
if(MSVC)
  target_compile_options(stb_image PRIVATE "/W0")
else()
  target_compile_options(stb_image PRIVATE "-w")
endif()


# --- Embree ------------------------------------------------------------------
set(EMBREE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/embree-4.4.1)
set(EMBREE_LIB_DIR ${EMBREE_ROOT}/lib64)

if(NOT EXISTS ${EMBREE_LIB_DIR}/libembree4.so)
  message(FATAL_ERROR
    "Embree not found at ${EMBREE_LIB_DIR}\n"
    "Download the x86_64 Linux release from\n"
    "  https://github.com/RenderKit/embree/releases\n"
    "and extract it to ${EMBREE_ROOT}")
endif()

add_library(embree INTERFACE)
target_include_directories(embree SYSTEM INTERFACE ${EMBREE_ROOT}/include)
target_link_libraries(embree INTERFACE ${EMBREE_LIB_DIR}/libembree4.so)