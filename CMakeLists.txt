cmake_minimum_required(VERSION 3.26)
project(redis_server)

set(CMAKE_CXX_STANDARD 23)

add_executable(redis_server server.cpp
        client.cpp)
