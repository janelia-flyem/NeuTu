cmake_minimum_required(VERSION 2.8)

PROJECT(FlyEM)

if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()


ADD_SUBDIRECTORY(src bin)


