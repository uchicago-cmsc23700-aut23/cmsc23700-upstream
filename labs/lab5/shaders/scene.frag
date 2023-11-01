/*! \file scene.vsh
 *
 * Lab 5 sample code: fragment shader for the scene.
 *
 * \author John Reppy
 */

/* CMSC23700 Sample code
 *
 * COPYRIGHT (c) 2023 John Reppy (http://www.cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#version 460

/** hard coded illumination levels **/
const vec3 ambLightIllum = vec3(0.15f, 0.15f, 0.15f);
const vec3 lightIllum = vec3(0.85f, 0.85f, 0.85f);
const float shadowEffect = 0.25;

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
layout (set = 0, binding = 1) uniform sampler2D colorMap;

layout (set = 1, binding = 0) uniform sampler2D shadowMap;

layout (location = 0) in vec4 fShadowPos; ///< shadow coordinate
layout (location = 1) in vec3 fNorm;    ///< interpolated vertex normal in
                                        ///  world coordinates
layout (location = 2) in vec2 fTC;      ///< vertex tex coordinate

layout (location = 0) out vec4 fragColor;

void main ()
{
    // renormalize the normal vector
    vec3 norm = normalize(fNorm);

    // compute diffuse illumination
    float intensity = max(dot(ubo.lightDir, norm), 0.0);
    vec3 lightC = clamp(ambLightIllum + intensity * lightIllum, 0, 1);

    // surface color
    vec3 surfaceC;
    if (ubo.enableTexture != 0) {
        surfaceC = texture(colorMap, fTC).rgb;
    } else {
        surfaceC = ubo.color;
    }

    /** HINT: when shadowing is enabled, shadow test goes here */

    // the fragment color is the light color times the surface color
    fragColor = vec4(lightC * surfaceC, 1.0);

}
