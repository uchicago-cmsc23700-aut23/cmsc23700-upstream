/*! \file shader-uniforms.hpp
 *
 * Type definitions for shader uniform data.  These definitions should agree
 * with the declarations in the shader files.
 *
 * The uniforms are divided into two buffers.  The first, which is used
 * by all vertex shaders holds the camera and viewport-dependent information
 * (i.e., the model and project matrices).  The second is used by the
 * texture and normal-map renderers and holds the scene-dependent lighting
 * information.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _SHADER_UNIFORMS_HPP_
#define _SHADER_UNIFORMS_HPP_

#ifndef _CS237_HPP_
#include "cs237.hpp"
#endif

/// The layout of a uniform buffer for the camera and viewport-dependent
/// information used in the vertex shaders.  Because the camera and/or
/// viewport can change at runtime, we need one VertexUB per frame.
//
struct VertexUB {
    alignas(16) glm::mat4 viewM;        //!< world-to-camera-space view transform
    alignas(16) glm::mat4 P;            //!< projection transform
};

/// The layout of a uniform buffer for the scene-specific lighting information
/// that is used in the fragment shader.  Since the lighting information is
/// fixed for the scene, we only need one UBO (not one per frame).
struct FragUB {
    alignas(16) glm::vec3 ambLight;     //!< intensity of ambient light
    struct {
        alignas(16) glm::vec3 lightPos;     //!< vector pointing toward directional light
        alignas(16) glm::vec3 lightColor;   //!< intensity of light
        alignas(16) glm::vec3 lightAtten;   //!< k0, k1, and k2 as vec3
    } lights[4];
    alignas(4) int nLights;             //!< the number of lights
};

/// a tuple of the information needed to support per-frame uniform-buffer
/// objects.  It is parameterized over the representation of the uniform
/// data.
//
template <typename UB>
struct UBOInfo {
    /// the type of uniform-buffer objects holding a `UB` value
    using UBO_t = cs237::UniformBuffer<UB>;

    bool valid;                     ///< true when the contents of the UBO is
                                    ///  valid (i.e., equal to the _ubCache)
    UBO_t *ubo;                     ///< the uniform buffer object
    vk::DescriptorSet descSet;      ///< the descriptor set for access to the UBO

    /// default constructor
    UBOInfo () : valid(false), ubo(nullptr), descSet() { }

    /// constructor
    UBOInfo (UBO_t *u, vk::DescriptorSet ds)
      : valid(false), ubo(u), descSet(ds)
    { }

    /// update the contents of the UBO and mark this as valid
    void update (UB const &ub)
    {
        this->ubo->copyTo(ub);
        this->valid = true;
    }

};

/// the types of the uniform buffer objects
using VertexInfo = UBOInfo<VertexUB>;
using VertexUBO_t = VertexInfo::UBO_t;
using FragInfo = UBOInfo<FragUB>;
using FragUBO_t = FragInfo::UBO_t;

/// Per-instance data, which we communicate using push constants
struct PushConsts {
    /* stuff for vertex shaders */
    alignas(16) glm::mat4 toWorld;      //!< model transform maps to world space
    alignas(16) glm::mat3 normToWorld;  //!< model transform for normal vectors
    /* shading support */
    alignas(16) glm::vec3 color;        //!< uniform color for object
};

#endif // !_SHADER_UNIFORMS_HPP_
