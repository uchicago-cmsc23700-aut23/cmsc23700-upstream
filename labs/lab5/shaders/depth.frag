/*! \file depth.frag
 *
 * Lab 5 sample code: fragment shader for the rendering the depth values for the light.
 *
 * \author John Reppy
 */

/* CMSC23700 Sample code
 *
 * COPYRIGHT (c) 2023 John Reppy (http://www.cs.uchicago.edu/~jhr)
 * All rights reserved.
 */
#version 460

layout (location = 0) out vec4 fragColor;

void main ()
{
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
