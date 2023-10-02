# CMake configuration for lab assignment 1 sources
#
# CMSC 23700 -- Introduction to Computer Graphics
# Autumn 2023
# University of Chicago
#
# COPYRIGHT (c) 2023 John Reppy
# All rights reserved.
#

set(TARGET lab1)

set(SRCS
  main.cpp)

add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} cs237)
add_dependencies(${TARGET} Lab1Shaders)
