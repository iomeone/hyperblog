cmake_minimum_required(VERSION 3.10)
project(hyperblog)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
link_directories("/usr/lib64/mysql")
link_libraries(mysqlclient)

link_libraries(jsoncpp)
link_libraries(pthread)
link_libraries(gtest)
#link_libraries(gmock)
link_libraries(gtest_main)
link_libraries(hiredis)
link_libraries(colib)
link_libraries(dl)

add_executable(hyperblog src/server/server.cpp)
#add_executable(hyperblog test/test.cpp)