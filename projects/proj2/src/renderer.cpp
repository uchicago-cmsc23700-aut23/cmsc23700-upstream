/*! \file renderer.cpp
 *
 * A Renderer bundles together the Vulkan renderpass and pipeline
 * objects for a particular shading mode.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "shader-uniforms.hpp"
#include "vertex.hpp"
#include "renderer.hpp"
#include "app.hpp"
#include "instance.hpp"
#include <string_view>

/// path to directory that holds the shaders
constexpr std::string_view kShaderDir = CS237_BINARY_DIR "/projects/proj2/shaders/";

Renderer::~Renderer ()
{
    this->_device().destroyPipeline(this->_pipeline);
    this->_device().destroyPipelineLayout(this->_pipelineLayout);
}

void Renderer::_initPipeline (
    RenderMode mode,
    std::vector<vk::DescriptorSetLayout> const &dsLayouts)
{
  /** HINT: here you need to create the pipeline for the renderer based on its mode.
   ** We recommend that you define a table of properties (e.g., shader names etc.)
   ** that is indexed by the render mode.
   **/
}


/******************** class WireframeRenderer ********************/

WireframeRenderer::~WireframeRenderer () { }

/// bind the descriptor sets for rendering a frame
/// \param cmdBuf   the command buffer to store the bind command in
/// \param vertUBO  the vertex-shader uniform-buffer-object information for
///                 the frame being rendered
/// \param fragUBO  the fragment-shader uniform-buffer-object information for
///                 the frame being rendered
void WireframeRenderer::bindFrameDescriptorSets (
    vk::CommandBuffer cmdBuf,
    VertexInfo const &vertUBO,
    FragInfo const &fragUBO)
{
    /** HINT: bind the vertex uniform buffer descriptor set */
}

/// bind the descriptor sets for rendering a given object
/// \param cmdBuf   the command buffer to store the bind command in
/// \param vertUBO  the vertex-shader uniform-buffer-object information for
///                 the frame being rendered
/// \param inst     the instance to be rendered using the UBO
void WireframeRenderer::bindMeshDescriptorSets (vk::CommandBuffer cmdBuf, Instance *inst)
{
    /* no texturing in wireframe mode, so nothing to do */
}


/******************** class FlatRenderer ********************/

FlatRenderer::~FlatRenderer () { }

/// bind the descriptor sets for rendering a frame
/// \param cmdBuf   the command buffer to store the bind command in
/// \param vertUBO  the vertex-shader uniform-buffer-object information for
///                 the frame being rendered
/// \param fragUBO  the fragment-shader uniform-buffer-object information for
///                 the frame being rendered
void FlatRenderer::bindFrameDescriptorSets (
    vk::CommandBuffer cmdBuf,
    VertexInfo const &vertUBO,
    FragInfo const &fragUBO)
{
    /** HINT: bind the vertex uniform buffer descriptor set */
}

/// bind the descriptor sets for rendering a given object
/// \param cmdBuf   the command buffer to store the bind command in
/// \param vertUBO  the vertex-shader uniform-buffer-object information for
///                 the frame being rendered
/// \param inst     the instance to be rendered using the UBO
void FlatRenderer::bindMeshDescriptorSets (vk::CommandBuffer cmdBuf, Instance *inst)
{
    /* no texturing in flat mode, so nothing to do */
}


/******************** class TextureRenderer ********************/

TextureRenderer::~TextureRenderer () { }

/// bind the descriptor sets for rendering a frame
/// \param cmdBuf   the command buffer to store the bind command in
/// \param vertUBO  the vertex-shader uniform-buffer-object information for
///                 the frame being rendered
/// \param fragUBO  the fragment-shader uniform-buffer-object information for
///                 the frame being rendered
void TextureRenderer::bindFrameDescriptorSets (
    vk::CommandBuffer cmdBuf,
    VertexInfo const &vertUBO,
    FragInfo const &fragUBO)
{
    /** HINT: bind the vertex and fragment shader uniform buffer descriptor sets */
}

/// bind the descriptor sets for rendering a given object
/// \param cmdBuf   the command buffer to store the bind command in
/// \param vertUBO  the vertex-shader uniform-buffer-object information for
///                 the frame being rendered
/// \param inst     the instance to be rendered using the UBO
void TextureRenderer::bindMeshDescriptorSets (vk::CommandBuffer cmdBuf, Instance *inst)
{
    /** HINT: bind the sampler descriptor sets */
}

/******************** class NormalMapRenderer ********************/

NormalMapRenderer::~NormalMapRenderer () { }

/// bind the descriptor sets for rendering a frame
/// \param cmdBuf   the command buffer to store the bind command in
/// \param vertUBO  the vertex-shader uniform-buffer-object information for
///                 the frame being rendered
/// \param fragUBO  the fragment-shader uniform-buffer-object information for
///                 the frame being rendered
void NormalMapRenderer::bindFrameDescriptorSets (
    vk::CommandBuffer cmdBuf,
    VertexInfo const &vertUBO,
    FragInfo const &fragUBO)
{
    /** HINT: bind the vertex and fragment shader uniform buffer descriptor sets */
}

/// bind the descriptor sets for rendering a given object
/// \param cmdBuf   the command buffer to store the bind command in
/// \param vertUBO  the vertex-shader uniform-buffer-object information for
///                 the frame being rendered
/// \param inst     the instance to be rendered using the UBO
void NormalMapRenderer::bindMeshDescriptorSets (vk::CommandBuffer cmdBuf, Instance *inst)
{
    /** HINT: bind the sampler descriptor sets */
}
