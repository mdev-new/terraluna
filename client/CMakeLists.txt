cmake_minimum_required(VERSION 3.15)
project(tlclient)

add_executable(tlclient Main.cc)
target_link_libraries(tlclient
    Assets
    grap
)