cmake_minimum_required(VERSION 3.15)
project(Assets)

set(CMAKE_CXX_STANDARD 17)

add_library(Assets STATIC AssetData.cc AssetData.hh TileData.hh)
target_include_directories(Assets PUBLIC .)