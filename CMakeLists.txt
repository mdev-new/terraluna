cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(Terraluna VERSION 0.2.0 DESCRIPTION "Awesome MMO RPG Game!")

# Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Binaries
set(LIBRARY_OUTPUT_PATH "bin/lib/")
set(LIBRARY_OUTPUT_DIRECTORY "bin/lib/")
set(EXECUTABLE_OUTPUT_PATH "bin/")

# Dependencies
add_subdirectory(vendor/glfw)

# Graphics
add_subdirectory(grap)

# Assets
add_subdirectory(assets)

# Server

# Client
add_subdirectory(client)