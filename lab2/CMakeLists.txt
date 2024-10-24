cmake_minimum_required(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
project(OSLab C)

set(CMAKE_C_STANDARD ${CMAKE_LANGUAGE_VERSION})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

add_executable(${PROJECT_NAME} main.c
        include/functions.h)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} m)