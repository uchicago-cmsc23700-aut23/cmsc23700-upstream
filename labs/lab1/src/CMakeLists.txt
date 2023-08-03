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

# path to include files
include_directories(
  ${GLFW_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${VULKAN_INCLUDE_DIR}
  ${CS237_INCLUDE_DIR})

add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} cs237)
target_link_libraries(${TARGET} ${GLFW_LIBRARY})
target_link_libraries(${TARGET} ${VULKAN_LIBRARY})
add_dependencies(${TARGET} Shaders)
