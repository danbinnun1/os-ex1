cmake_minimum_required(VERSION 3.0.0)
project(ex1 VERSION 0.1.0)

add_executable(ex1 ex1.c)
add_compile_options(-Wall -Wextra -Wshadow -pedantic)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
