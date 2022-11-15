cmake_minimum_required(VERSION 3.18.4)
project(profile)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(profile
        main.cpp
        )

target_link_libraries(profile ${OpenCV_LIBS})