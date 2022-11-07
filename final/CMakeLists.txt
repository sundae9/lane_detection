cmake_minimum_required(VERSION 3.18.4)
project(final)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(final
        main.cpp
        )

target_link_libraries(final ${OpenCV_LIBS})