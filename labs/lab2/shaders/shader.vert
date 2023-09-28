/*! \file no-light.vert
 *
 * Simple vertex shader without lighting or transforms
 *
 * \author John Reppy
 */

/* CMSC23700 Project 1 sample code (Autumn 2023)
 *
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#version 460

layout (location = 0) in vec2 vPos;		//!< input vertex  position
layout (location = 1) in vec3 vColor;		//!< input vertex color
layout (location = 0) out vec3 fragColor;	//!< output fragment color

void main ()
{
    gl_Position = vec4(vPos, 0.0, 1.0);
    fragColor = vColor;
}
