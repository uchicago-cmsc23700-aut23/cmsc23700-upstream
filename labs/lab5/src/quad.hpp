/*! \file quad.hpp
 *
 * CMSC 23700 Autumn 2023 Lab 5.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _QUAD_HPP_
#define _QUAD_HPP_

#include "cs237.hpp"

/// A quad with a normal vector to define its front face.
struct Quad {
    glm::vec3 norm;             ///< the normal vector that defines the "front"
    glm::vec3 corner[4];        ///< the corners of the quad in CCW order
    glm::vec2 texCoord[4];      ///< the texture coordinates of the quad in CCW order

    // return indices of the two triangles that comprise the quad in CCW order
    constexpr static const std::array<uint16_t,6> indices = { 0, 1, 2, 2, 3, 0 };
};

#endif /* _QUAD_HPP_ */
