/*! \file drawable.hpp
 *
 * CMSC 23700 Autumn 2023 Lab 5.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _DRAWABLE_HXX_
#define _DRAWABLE_HXX_

#include "mesh.hpp"
#include "uniforms.hpp"

/// the information that we need to draw stuff
struct Drawable {
    vk::Device device;                  ///< the owning device (needed for cleanup)
    cs237::VertexBuffer<Vertex> *vBuf;  ///< vertex buffer for mesh vertices
    cs237::IndexBuffer<uint16_t> *iBuf; ///< index buffer for mesh indices
    glm::mat4 modelMat;                 ///< drawable's model matrix
    glm::vec3 color;                    ///< drawable's color
    cs237::Texture2D *tex;              ///< drawable's texture
    vk::DescriptorSet descSet;          ///< descriptor set for the texture sampler
    UBO_t *ubo;                         ///< the uniform buffer for the drawable
    vk::Sampler sampler;                ///< the texture sampler

    Drawable (cs237::Application *app, const Mesh *mesh);
    ~Drawable ();

    /// allocate the descriptor sets for the drawable
    /// \param dsPool        the descriptor pool used for allocation
    /// \param dsLayout      the layout of the descriptor sets
    void initDescriptors (vk::DescriptorPool dsPool, vk::DescriptorSetLayout dsLayout);

    /// update the UBO
    /// \param worldToLight  the world-space to light-space transform matrix
    /// \param ub            a UB struct that holds the non-model-dependent
    ///                      uniform values
    void updateUBO (glm::mat4 const &worldToLight, UB &ub)
    {
        // set the per-model fields
        ub.modelMat = this->modelMat;
        ub.shadowMat = worldToLight * this->modelMat;
        ub.color = this->color;
        // copy to the GPU
        this->ubo->copyTo(ub);
    }

    /// bind the sampler descriptor sets for the drawable as Set 0
    void bindDescriptorSets (
        vk::CommandBuffer cmdBuf,
        vk::PipelineLayout pipeLayout);

    void draw (vk::CommandBuffer cmdBuf)
    {
        // bind the vertex buffer
        vk::Buffer vertBuffers[] = {this->vBuf->vkBuffer()};
        vk::DeviceSize offsets[] = {0};
        cmdBuf.bindVertexBuffers(0, vertBuffers, offsets);

        // bind the index buffer
        cmdBuf.bindIndexBuffer(this->iBuf->vkBuffer(), 0, vk::IndexType::eUint16);

        cmdBuf.drawIndexed(this->iBuf->nIndices(), 1, 0, 0, 0);
    }

};

#endif /* _DRAWABLE_HXX_ */
