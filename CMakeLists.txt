cmake_minimum_required(VERSION 3.28)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mik32_toolchain.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("YADRO project" C CXX ASM)
enable_language(C CXX ASM)

add_subdirectory(hardware)
add_subdirectory(libs)
add_subdirectory(sources)