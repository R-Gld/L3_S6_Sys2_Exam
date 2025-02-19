cmake_minimum_required(VERSION 3.31)
project(l3_s6_exam C)
set(CMAKE_C_STANDARD 11)

find_library(MATH_LIBRARY m)

add_executable(l3_s6_exam main.c)
target_link_libraries(l3_s6_exam ${MATH_LIBRARY})
