cmake_minimum_required(VERSION 3.18.4)
project(ver2_0)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(ver2_0
        main.cpp
        )

target_link_libraries(ver2_0 ${OpenCV_LIBS})