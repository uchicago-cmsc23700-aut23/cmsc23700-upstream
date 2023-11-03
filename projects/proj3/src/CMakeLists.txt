# CMake configuration for individual Project 3 sources
#
# CMSC 23700 -- Introduction to Computer Graphics
# Autumn 2023
# University of Chicago
#
# COPYRIGHT (c) 2023 John Reppy
# All rights reserved.
#

set(TARGET proj3)

set(SRCS
  app.cpp
  height-field.cpp
  main.cpp
  mesh.cpp
  renderer.cpp
  scene.cpp
  window.cpp)

add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} cs237)
add_dependencies(${TARGET} proj3-shaders)
