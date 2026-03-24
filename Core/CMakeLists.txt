cmake_minimum_required(VERSION 3.25)

project(Core)

file(GLOB_RECURSE CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(Core ${CORE_SOURCES})

target_include_directories(Core PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Network
)

# ASIO is used as standalone headers in this project
target_compile_definitions(Core PRIVATE ASIO_STANDALONE)

target_link_libraries(Core PRIVATE Interface)

if(UNIX)
  target_link_libraries(Core PRIVATE dl pthread)
endif()

set_target_properties(Core PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>
)
