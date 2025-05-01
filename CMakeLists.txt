cmake_minimum_required(VERSION 3.15)
project(editor)
set(CMAKE_CXX_STANDARD 20)
include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src srcs)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_executable(edit ${srcs})



