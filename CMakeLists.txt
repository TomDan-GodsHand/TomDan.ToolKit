cmake_minimum_required(VERSION 3.25)
project(TomDan.ToolKit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(USE_LIBCXX "Use libc++ and lld where available" ON)
if(USE_LIBCXX)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-stdlib=libc++)
    add_link_options(-stdlib=libc++)
    add_link_options(-fuse-ld=lld)
  endif()
endif()

add_subdirectory(Interface)
add_subdirectory(Core)
add_subdirectory(Plugin.Example)
