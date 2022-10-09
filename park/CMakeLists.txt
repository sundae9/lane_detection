cmake_minimum_required(VERSION 3.18.4)
project(park)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(park
        main.cpp
        #        videoPlayer.cpp
        timeRecord.cpp header/timeRecord.hpp)

target_link_libraries(park ${OpenCV_LIBS})