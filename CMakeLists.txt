cmake_minimum_required(VERSION 3.10)
project(SocketCommunication)

set(CMAKE_CXX_STANDARD 11)

add_executable(Server Server.cpp)
add_executable(Client Client.cpp)

