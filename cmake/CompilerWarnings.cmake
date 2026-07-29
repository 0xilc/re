add_library(re_warnings INTERFACE)
target_compile_options(re_warnings INTERFACE 
  -Wall -Wextra -Wshadow -Wconversion)