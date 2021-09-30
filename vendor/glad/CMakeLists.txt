cmake_minimum_required(VERSION 3.16)

add_library(
    glad
    "src/glad.cpp"
    "include/glad/glad.h"
)

target_include_directories(glad PUBLIC include)