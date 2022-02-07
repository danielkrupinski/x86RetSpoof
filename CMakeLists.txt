cmake_minimum_required(VERSION 3.16)
project(x86RetSpoof CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
  add_compile_options(/W4 /WX /w44388)
endif()

include_directories(.)

option(BUILD_TESTS "Build unit tests" OFF)

if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(Tests)
endif()
