cmake_minimum_required(VERSION 3.31)
project(tp_note_n1 C)
set(CMAKE_C_STANDARD 11)

find_library(MATH_LIBRARY m)

add_executable(tp_note_n1 main.c)
target_link_libraries(tp_note_n1 ${MATH_LIBRARY})
