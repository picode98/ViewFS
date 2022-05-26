cmake_minimum_required(VERSION 3.21)
project(ViewFS)

set(CMAKE_CXX_STANDARD 17)

add_executable(ViewFS main.cpp ProjFSBase.cpp WinUtils.cpp)
target_link_libraries(ViewFS ProjectedFSLib.lib)
