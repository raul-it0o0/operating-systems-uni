cmake_minimum_required(VERSION 3.28)
project(lab4 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

add_executable(lab4 main.c)

target_include_directories(lab4 PUBLIC include)