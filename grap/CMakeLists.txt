cmake_minimum_required(VERSION 3.15)
project(grap)

add_library(grap STATIC
    # Main Header
    Grap.hh Grap.cc
)

target_link_libraries(grap PUBLIC
    glfw
)
target_include_directories(grap PUBLIC .)