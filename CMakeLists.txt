cmake_minimum_required(VERSION 2.6)

project(stub)

enable_language(C)
enable_language(CXX)

include("defaults.cmake")

find_boost_program_options(TRUE)
find_glew(TRUE STATIC)
find_opengl(TRUE)
find_png(TRUE)
find_sdl(TRUE)

add_definitions(-DUSE_LD)

output_flags("DEBUG" on)

add_subdirectory(src)
