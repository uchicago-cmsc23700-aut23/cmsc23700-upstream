/*! \file scene.vert
 *
 * Lab 5 sample code: vertex shader for the scene.
 *
 * \author John Reppy
 */

/* CMSC23700 Sample code
 *
 * COPYRIGHT (c) 2023 John Reppy (http://www.cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#version 460

// matrix for mapping 2x2x1 clip coordinates to [0,1] range
const mat4 biasMat = mat4(
    0.5, 0.0, 0.0, 0.0,  // first column
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0);

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
layout (location = 1) in vec3 vNorm;    ///< vertex normal in model coordinates
layout (location = 2) in vec2 vTC;      ///< vertex texture coordinate

layout (location = 0) out vec4 fShadowPos; ///< shadow coordinate
layout (location = 1) out vec3 fNorm;   ///< interpolated vertex normal in
                                        ///  world coordinates
layout (location = 2) out vec2 fTC;     ///< vertex tex coordinate

void main ()
{
    // clip coordinates for vertex
    gl_Position = ubo.projMat * ubo.viewMat * ubo.modelMat * vec4(vPos, 1);

    // eye-space normal; we are assuming modelMat is orthogonal
    fNorm = mat3x3(ubo.modelMat) * vNorm;

    /** HINT: when shadowing is enabled, compute `fShadowPos` */

    // propagate texture coords to fragment shader
    fTC = vTC;
}
