cmake_minimum_required(VERSION 3.18.4)
project(lee)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(lee
        main.cpp
        )
target_link_libraries(lee ${OpenCV_LIBS})
