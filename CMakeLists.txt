cmake_minimum_required(VERSION 3.19)
project(ex4_OS C)

set(CMAKE_C_STANDARD 99)

add_executable(ex4_OS threadPool.c threadPool.h test.c)