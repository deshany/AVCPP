cmake_minimum_required(VERSION 3.22)
project(AVCPPServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(AVCPPServer main.cpp
        Core/VideoCapture.cpp
        Core/VideoCapture.h)
