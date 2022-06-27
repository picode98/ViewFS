cmake_minimum_required(VERSION 3.21)
project(ViewFS)

set(CMAKE_CXX_STANDARD 20)

add_executable(ViewFS main.cpp windows/ProjFSBase.cpp windows/WinUtils.cpp windows/NamedPipe.cpp
        PythonSubprocessSource.cpp windows/Subprocess.cpp Utils.cpp PlatformUtils.cpp windows/ProjFSFrontend.cpp
        windows/HardlinkFrontend.cpp)
target_link_libraries(ViewFS ProjectedFSLib.lib)
