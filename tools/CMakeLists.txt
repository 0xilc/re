add_executable(image_diff image_diff/main.cpp)
target_link_libraries(image_diff PRIVATE stb_image)

target_compile_options(image_diff PRIVATE -w)