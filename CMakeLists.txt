cmake_minimum_required(VERSION 3.10)
project(hyperblog)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
link_directories("/usr/lib64/mysql")
link_libraries(mysqlclient)

add_executable(hyperblog main.cpp Properties.hpp)
