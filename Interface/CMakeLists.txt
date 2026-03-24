cmake_minimum_required(VERSION 3.25)

project(Interface)

add_library(Interface INTERFACE)
target_include_directories(Interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
