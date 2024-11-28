cmake_minimum_required(VERSION 3.28)
project(lab8 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

add_executable(lab8 main.c)

target_include_directories(lab8 PUBLIC include)