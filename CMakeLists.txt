cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED True)

project("RTD_DualAlpha")

add_definitions(-DUNICODE -D_UNICODE)

include_directories(include)

add_executable (RTD_DualAlpha src/main.cpp)

target_include_directories(RTD_DualAlpha PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src")

