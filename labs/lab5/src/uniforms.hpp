/*! \file uniforms.hpp
 *
 * CMSC 23700 Autumn 2023 Lab 5.
 *
 * The layout of uniform data for the shaders.  We use the same layout
 * for all of the shaders.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _UNIFORMS_HPP_
#define _UNIFORMS_HPP_

struct UB {
    alignas(16) glm::mat4 modelMat;             ///< model matrix
    alignas(16) glm::mat4 viewMat;              ///< view matrix
    alignas(16) glm::mat4 projMat;              ///< projection matrix
    alignas(16) glm::mat4 shadowMat;            ///< model-space to light-space transform
    alignas(16) glm::vec3 lightDir;             ///< light direction in eye coordinates
    alignas(16) glm::vec3 color;                ///< flat color to use when texturing is disabled
    alignas(4) vk::Bool32 enableTexture;        ///< non-zero when texturing is enabled
    alignas(4) vk::Bool32 enableShadows;        ///< non-zero when shadows are enabled
};

using UBO_t = cs237::UniformBuffer<UB>;

#endif // !_UNIFORMS_HPP_
