# CMake configuration for individual project 2 sources
#
# CMSC 23700 -- Introduction to Computer Graphics
# Autumn 2023
# University of Chicago
#
# COPYRIGHT (c) 2023 John Reppy
# All rights reserved.
#

set(TARGET proj2)

set(SRCS
  app.cpp
  main.cpp
  mesh.cpp
  renderer.cpp
  scene.cpp
  window.cpp)

add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} cs237)
add_dependencies(${TARGET} proj2-shaders)
