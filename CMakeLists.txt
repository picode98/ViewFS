cmake_minimum_required(VERSION 3.21)
project(ViewFS)

set(CMAKE_CXX_STANDARD 17)

add_executable(ViewFS main.cpp windows/ProjFSBase.cpp windows/WinUtils.cpp windows/NamedPipe.cpp)
target_link_libraries(ViewFS ProjectedFSLib.lib)
