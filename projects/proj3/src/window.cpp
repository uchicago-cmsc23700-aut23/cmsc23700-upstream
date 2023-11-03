/*! \file window.cpp
 *
 * CS23700 Autumn 2023 Sample Code for Project 3
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "window.hpp"
#include "obj.hpp"
#include "render-modes.hpp"
#include "renderer.hpp"
#include "shader-uniforms.hpp"
#include "vertex.hpp"

/// constants to define the near and far planes of the view frustum
constexpr float kNearZ = 0.5;   // how close to the origin you can get
constexpr float kFarZ = 500.0f;  // distance to far plane

Proj3Window::Proj3Window (Proj3 *app)
  : cs237::Window (
        app,
        // resizable window with depth buffer
        cs237::CreateWindowInfo(
            app->scene()->width(),
            app->scene()->height(),
            "", true, true, false)),
    _mode(RenderMode::eTextureShading),
    _syncObjs(this)
{
    // initialize the camera from the scene
    this->_camPos = app->scene()->cameraPos();
    this->_camAt = app->scene()->cameraLookAt();
    this->_camUp = app->scene()->cameraUp();

    /** HINT: add additional initialization for render modes */

    this->_initMeshes(app->scene());

    this->_initRenderPass ();

    // create framebuffers for the swap chain
    this->_swap.initFramebuffers (this->_renderPass);

    /** HINT: add additional initialization for uniform buffers and renderers */

    // create the command buffer
    this->_cmdBuffer = app->newCommandBuf();

    // enable handling of keyboard events
    this->enableKeyEvent (true);
}

Proj3Window::~Proj3Window ()
{
    auto device = this->device();

    /* delete the command buffer */
    this->_app->freeCommandBuf (this->_cmdBuffer);

    device.destroyRenderPass(this->_renderPass);
    device.destroyDescriptorPool(this->_descPool);
    device.destroyDescriptorSetLayout(this->_vertDSLayout);
    device.destroyDescriptorSetLayout(this->_fragDSLayout);

    for (auto it : this->_vertUBOs) {
        delete it.ubo;
    }
    delete this->_fragUBO.ubo;

    /** HINT: release any other allocated objects */

}

void Proj3Window::_initRenderPass ()
{
    // initialize the attachment descriptors and references
    std::vector<vk::AttachmentDescription> atDescs;
    std::vector<vk::AttachmentReference> atRefs;
    this->_initAttachments (atDescs, atRefs);
    assert (atRefs.size() == 2);  /* expect a depth buffer */

    vk::SubpassDescription subpass(
        {}, /* flags */
        vk::PipelineBindPoint::eGraphics, /* pipeline bind point */
        0, nullptr, /* input attachments */
        1, &(atRefs[0]), /* color attachments */
        nullptr, /* resolve attachments */
        &(atRefs[1]), /* depth-stencil attachment */
        0, nullptr); /* preserve attachments */

    vk::SubpassDependency dependency(
        VK_SUBPASS_EXTERNAL, /* src subpass */
        0, /* dst subpass */
        vk::PipelineStageFlagBits::eColorAttachmentOutput, /* src stage mask */
        vk::PipelineStageFlagBits::eColorAttachmentOutput, /* dst stage mask */
        {}, /* src access mask */
        { vk::AccessFlagBits::eColorAttachmentWrite }, /* dst access mask */
        {}); /* dependency flags */

    vk::RenderPassCreateInfo renderPassInfo(
        {}, /* flags */
        atDescs, /* attachments */
        subpass, /* subpasses_ */
        dependency); /* dependencies */

    this->_renderPass = this->device().createRenderPass(renderPassInfo);

}

void Proj3Window::_initUBOs ()
{
    auto device = this->device();

    // create the descriptor pool; we have one descriptor per frame plus one for the
    // per-scene fragment-shader uniforms
    int nSets = this->_swap.size() + 1;
    vk::DescriptorPoolSize poolSz(
        vk::DescriptorType::eUniformBuffer,
        nSets+1);
    vk::DescriptorPoolCreateInfo poolInfo(
        {}, /* flags */
        nSets+1, /* max sets */
        poolSz); /* pool sizes */
    this->_descPool = device.createDescriptorPool(poolInfo);

    /** HINT: create the vertex-shader and fragment-shader descriptor
     ** layouts for the uniform buffers.
     **/

    /** HINT: allocate the uniform buffers and descriptors. */

    /** HINT: initialize the fragment-shader uniform buffer. */
}

void Proj3Window::_initRenderers ()
{
    Proj3 *app = reinterpret_cast<Proj3 *>(this->_app);

    // create the renderers
    /** HINT: create the renderer objects for each of the modes here */

}

void Proj3Window::_initMeshes (const Scene *scene)
{
    /** HINT: put code to construct the meshes and instances
     *  from the scene here
     */
}

void Proj3Window::_recordCommandBuffer (uint32_t imageIdx)
{
    Renderer *rp = nullptr; /** HINT: set `rp` to the current renderer */

    vk::CommandBufferBeginInfo beginInfo;
    this->_cmdBuffer.begin(beginInfo);

    std::array<vk::ClearValue,2> clearValues = {
            vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f), /* clear the window to black */
            vk::ClearDepthStencilValue(1.0f, 0.0f)
        };
    vk::RenderPassBeginInfo renderPassInfo(
        this->_renderPass,
        this->_swap.fBufs[imageIdx],
        { {0, 0}, this->_swap.extent }, /* render area */
        clearValues);

    this->_cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    /*** BEGIN COMMANDS ***/

    // set the viewport using the OpenGL convention
    this->_setViewportCmd (this->_cmdBuffer, true);

    rp->bindPipelineCmd(this->_cmdBuffer);

    // bind per-frame descriptors
    rp->bindFrameDescriptorSets (
        this->_cmdBuffer,
        this->_vertUBOs[imageIdx],
        this->_fragUBO);

    // render the objects in the scene
    for (auto it : this->_objs) {
        // bind the descriptors for the object
        rp->bindMeshDescriptorSets (this->_cmdBuffer, it);

        /** HINT: set the push constants for the mesh */

        it->mesh->draw (this->_cmdBuffer);
    }

    /*** END COMMANDS ***/

    this->_cmdBuffer.endRenderPass();

    this->_cmdBuffer.end();

}

void Proj3Window::draw ()
{
    // next buffer from the swap chain
    auto imageIndex = this->_syncObjs.acquireNextImage ();
    if (imageIndex.result != vk::Result::eSuccess) {
        ERROR("inable to acquire next image");
    }
    int idx = imageIndex.value;

    this->_syncObjs.reset();

    /** HINT: update the UBO, if necessary */

    this->_cmdBuffer.reset();
    this->_recordCommandBuffer (idx);

    // set up submission for the graphics queue
    this->_syncObjs.submitCommands (this->graphicsQ(), this->_cmdBuffer);

    // set up submission for the presentation queue
    this->_syncObjs.present (this->presentationQ(), idx);
}

void Proj3Window::reshape (int wid, int ht)
{
    // invoke the super-method reshape method
    this->cs237::Window::reshape(wid, ht);

    // recreate the new framebuffers
    this->_swap.initFramebuffers (this->_renderPass);

    /** HINT: update the UBO cache */

}

void Proj3Window::key (int key, int scancode, int action, int mods)
{
  // ignore releases, control keys, command keys, etc.
    if ((action != GLFW_RELEASE)
    || (mods & (GLFW_MOD_CONTROL|GLFW_MOD_ALT|GLFW_MOD_SUPER))) {

        switch (key) {
            case GLFW_KEY_N:  // 'n' or 'n' ==> switch to normal-mapping mode
                this->_mode = RenderMode::eNormalMapShading;
                break;
            case GLFW_KEY_S:  // 's' or 'S' ==> toggle shadows
                /** HINT: toggle the shadow state here */
                break;
            case GLFW_KEY_T:  // 't' or 'T' ==> switch to texturing mode
                this->_mode = RenderMode::eTextureShading;
                break;
            case GLFW_KEY_Q:  // 'q' or 'Q' ==> quit
                glfwSetWindowShouldClose (this->_win, true);
                break;

/** HINT: add cases for camera controls */

            default: // ignore all other keys
                return;
        }
    }

}

