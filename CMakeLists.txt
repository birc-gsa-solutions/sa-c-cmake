cmake_minimum_required (VERSION 3.21)
project (sa)
enable_testing()

add_executable(sa src/sa.c)
add_test(sa sa)

