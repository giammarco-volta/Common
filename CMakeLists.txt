cmake_minimum_required(VERSION 3.21)

project(NaadaLabCommon
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core)

qt_standard_project_setup(REQUIRES 6.8)

add_subdirectory(src/midi)