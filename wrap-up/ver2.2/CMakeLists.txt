cmake_minimum_required(VERSION 3.18.4)
project(ver2_2)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(ver2_2
        main.cpp
        )

target_link_libraries(ver2_2 ${OpenCV_LIBS})