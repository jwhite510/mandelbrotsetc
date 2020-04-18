cmake_minimum_required(VERSION 3.0)
project(projectname)

add_definitions("-std=c++11")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
message(${CMAKE_MODULE_PATH})

find_package(SFML 2 REQUIRED COMPONENTS network audio graphics window system)
add_executable(a.out src/main.cpp)
target_link_libraries(a.out ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})

