/*! \file window.hpp
 *
 * CS23700 Autumn 2023 Sample Code for Project 1
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _WINDOW_HPP_
#define _WINDOW_HPP_

#include "cs237.hpp"
#include "app.hpp"
#include "instance.hpp"
#include "render-modes.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "shader-uniforms.hpp"

/// The Project 1 Window class
class Proj1Window : public cs237::Window {
public:
    Proj1Window (Proj1 *app);

    ~Proj1Window () override;

    void draw () override;

    /// handle keyboard events
    void key (int key, int scancode, int action, int mods) override;

private:
    vk::RenderPass _renderPass;                 ///< the shared render pass for drawing
    RenderMode _mode;                           ///< the current rendering mode
    vk::CommandBuffer _cmdBuffer;               ///< the command buffer
    SyncObjs _syncObjs;                         ///< synchronization objects for the
                                                ///  swap chain

    // support for uniform buffers
    vk::DescriptorPool _descPool;               ///< the descriptor pool
    vk::DescriptorSetLayout _dsLayout;          ///< layout of descriptor set
    /** HINT: you will need an array (or arrays) of information to support the per-frame
     ** uniform buffers.
     **/

    // scene data
    std::vector<Mesh *> _meshes;                ///< the meshes in the scene
    std::vector<Instance *> _objs;              ///< the objects to render

    // Current camera state
    glm::vec3 _camPos;                          ///< camera position in world space
    glm::vec3 _camAt;                           ///< camera look-at point in world space
    glm::vec3 _camUp;                           ///< camera up vector in world space
    SceneUB _ubCache;                           ///< cache of scene uniform data; this
                                                ///  cache gets updated when the camera
                                                ///  state changes

    /// allocate and initialize the meshes and drawables
    void _initMeshes (const Scene *scene);

    /// initialize the `_renderPass` field
    void _initRenderPass ();

    /** HINT: you will need to define initialization functions
     ** to initialize the rendering structures, uniforms, etc.
     */

    /// record the rendering commands
    void _recordCommandBuffer (uint32_t imageIdx);

    /// get the scene being rendered
    const Scene *_scene () const
    {
        return reinterpret_cast<Proj1 *>(this->_app)->scene();
    }

}; // Proj1Window

#endif // !_WINDOW_HPP_
