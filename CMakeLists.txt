cmake_minimum_required(VERSION 3.6)
project(OLisp)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c mpc.h mpc.c builtin.c lval.c)
add_executable(OLisp ${SOURCE_FILES})
target_link_libraries(OLisp edit)
