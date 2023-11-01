/*! \file depth.vert
 *
 * Lab 5 sample code: vertex shader for the rendering the depth values for the light.
 *
 * \author John Reppy
 */

/* CMSC23700 Sample code
 *
 * COPYRIGHT (c) 2023 John Reppy (http://www.cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#version 460

layout (set = 0, binding = 0) uniform UB {
    mat4 modelMat;      ///< model matrix
    mat4 viewMat;       ///< view matrix
    mat4 projMat;       ///< projection matrix
    mat4 shadowMat;     ///< model-space to light-space transform
    vec3 lightDir;      ///< light direction in eye coordinates
    vec3 color;         ///< flat color to use when texturing is disabled
    int enableTexture;  ///< non-zero when texturing is enabled
    int enableShadows;  ///< non-zero when shadows are enabled
} ubo;

layout (location = 0) in vec3 vPos;	///< vertex position in model coordinates

void main ()
{
    // light clip coordinates
    gl_Position = ubo.shadowMat * vec4(vPos, 1);
}
