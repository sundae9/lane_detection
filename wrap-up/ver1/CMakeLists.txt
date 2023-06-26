cmake_minimum_required(VERSION 3.18.4)
project(ver1)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(ver1
        main.cpp
        )

target_link_libraries(ver1 ${OpenCV_LIBS})