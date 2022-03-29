cmake_minimum_required(VERSION 3.10)
project(FlashGC)

set(CMAKE_CXX_STANDARD 11)

add_executable(FlashGC main.cpp main.hpp FTL.hpp ListItem.h Auxilaries.h Auxilaries.cpp AlgoRunner.h)