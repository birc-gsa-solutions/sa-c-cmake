cmake_minimum_required(VERSION 3.10)

project(cstr)
set(CMAKE_C_STANDARD 23)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  include(CTest)
  set(GEN_UNIT_TESTS 1)
  add_subdirectory(test)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

add_subdirectory(cstr)
add_subdirectory(test)

