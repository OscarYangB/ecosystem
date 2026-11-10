cmake_minimum_required(VERSION 3.22)
project(ecosystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV 5.0 REQUIRED)

add_subdirectory("external/SDL")

message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")

add_executable(opencv_test_app main.cpp platform_render.cpp grid.cpp render.cpp)
target_include_directories(opencv_test_app PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(opencv_test_app PRIVATE ${OpenCV_LIBS} SDL3::SDL3)