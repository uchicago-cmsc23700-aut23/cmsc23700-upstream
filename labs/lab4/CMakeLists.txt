# CMake configuration for lab assignment 4
#
# CMSC 23700 -- Introduction to Computer Graphics
# Autumn 2023
# University of Chicago
#
# COPYRIGHT (c) 2023 John Reppy
# All rights reserved.
#

project(CMSC237_LAB_4
  VERSION 1
  HOMEPAGE_URL "https://classes.cs.uchicago.edu/archive/2023/fall/23700-1/labs/lab4/index.html"
  LANGUAGES C CXX)

# shader source code
#
add_subdirectory(shaders)

# the source code for the project
#
add_subdirectory(src)
