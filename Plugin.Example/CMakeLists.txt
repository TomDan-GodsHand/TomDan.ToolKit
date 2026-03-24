cmake_minimum_required(VERSION 3.25)

project(Plugin.Example)

file(GLOB PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(Plugin.Example SHARED ${PLUGIN_SOURCES})
target_compile_definitions(Plugin.Example PRIVATE PLUGIN_EXPORTS)
target_link_libraries(Plugin.Example PRIVATE Interface)

set_target_properties(Plugin.Example PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>/plugins
)
