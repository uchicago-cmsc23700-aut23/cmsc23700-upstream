# CMake configuration for individual project 2
#
# CMSC 23700 -- Introduction to Computer Graphics
# Autumn 2023
# University of Chicago
#
# COPYRIGHT (c) 2023 John Reppy
# All rights reserved.
#

project(CMSC237_PROJ_2
  VERSION 1
  HOMEPAGE_URL "https://classes.cs.uchicago.edu/archive/2023/fall/23700-1/index.html"
  LANGUAGES C CXX)

# shader source code
#
add_subdirectory(shaders)

# the source code for the project
#
add_subdirectory(src)
