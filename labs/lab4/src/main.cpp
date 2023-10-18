/*! \file main.cpp
 *
 * CMSC 23700 Autumn 2023 Lab 4.  This file is the main program
 * for Lab4.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hpp"
#include "mesh.hpp"

#ifdef CS237_BINARY_DIR
//!< the absolute path to the directory containing the compiled shaders
const std::string kShaderDir = CS237_BINARY_DIR "/labs/lab4/shaders/";
#else
# error CS237_BINARY_DIR not defined
#endif

// view parameters; these are constants for now.
static const float kNearZ = 0.2f;       //!< distance to near plane
static const float kFarZ = 100.0f;      //!< distance to far plane
static const float kFOV = 90.0f;        //!< field of view angle in degrees

// layout of the unform buffer for the vertex shader; we use the `alignas`
// annotations to ensure that the values are correctly aligned.  See
// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout
// for details on the alignment requirements.
//
struct UB {
    alignas(16) glm::mat4 MV;           //!< model-view transform
    alignas(16) glm::mat4 P;            //!< projection transform
    alignas(16) glm::vec4 color;        //! the uniform color of the cube
};

using UBO_t = cs237::UniformBuffer<UB>;


/******************** derived classes ********************/

//! The Lab 2 Application class
class Lab4 : public cs237::Application {
public:
    Lab4 (std::vector<std::string> const &args);
    ~Lab4 ();

    void run () override;

private:
    class Lab4Window *_win;
};

//! The Lab 2 Window class
class Lab4Window : public cs237::Window {
public:
    Lab4Window (Lab4 *app);

    ~Lab4Window () override;

    /// reshape the window
    void reshape (int wid, int ht) override;

    void draw () override;

    //! handle keyboard events
    void key (int key, int scancode, int action, int mods) override;

private:
    vk::RenderPass _renderPass;
    vk::PipelineLayout _pipelineLayout;
    vk::Pipeline _graphicsPipeline;
    vk::CommandBuffer _cmdBuf;
    cs237::VertexBuffer<Vertex> *_vertBuffer;   ///< vertex buffer for cube vertices
    cs237::IndexBuffer<uint16_t> *_idxBuffer;   ///< index buffer for cube indices
    UBO_t *_ubo;                                //!< uniform buffer for vertex shader
    vk::DescriptorSetLayout _descSetLayout;     ///< descriptor-set layout for uniform buffer
    vk::DescriptorPool _descPool;               ///< descriptor-set pool
    vk::DescriptorSet _descSet;                 ///< descriptor set for uniform buffer
    SyncObjs _syncObjs;
    // texture state
    cs237::Texture2D *_txt;                     //!< the texture image etc.
    vk::Sampler _txtSampler;                    //!< the texture sampler
    // Camera state
    glm::vec3 _camPos;                          //!< camera position in world space
    glm::vec3 _camAt;                           //!< camera look-at point in world space
    glm::vec3 _camUp;                           //!< camera up vector in world space

    //! allocate and initialize the uniforms
    void _allocUniforms ();

    //! initialize the uniform buffer object using model, view, and projection
    //! matrices that are computed from the current camera state.
    void _initUniforms ();

    //! initialize the UBO descriptors
    void _initDescriptors ();
    //! initialize the `_renderPass` field
    void _initRenderPass ();
    //! initialize the `_pipelineLayout` and `_graphicsPipeline` fields
    void _initPipeline ();
    //! allocate and initialize the data buffers
    void _initData ();
    //! record the rendering commands
    void _recordCommandBuffer (uint32_t imageIdx);

};

/******************** Lab4Window methods ********************/

Lab4Window::Lab4Window (Lab4 *app)
  : cs237::Window (
        app,
        // resizable window with depth buffer and no stencil
        cs237::CreateWindowInfo(800, 600, app->name(), true, true, false)),
    _syncObjs(this)
{
    // create the texture sampler
    cs237::Application::SamplerInfo samplerInfo(
        vk::Filter::eLinear,                    // magnification filter
        vk::Filter::eLinear,                    // minification filter
        vk::SamplerMipmapMode::eLinear,         // mipmap mode
        vk::SamplerAddressMode::eClampToEdge,   // addressing mode for U coordinates
        vk::SamplerAddressMode::eClampToEdge,   // addressing mode for V coordinates
        vk::BorderColor::eIntOpaqueBlack);      // border color
    this->_txtSampler = this->_app->createSampler (samplerInfo);

    // initialize the camera
    this->_camPos = glm::vec3(0.0f, 0.0f, 4.0f);
    this->_camAt = glm::vec3(0.0f, 0.0f, 0.0f);
    this->_camUp = glm::vec3(0.0f, 1.0f, 0.0f);

    this->_initData();
    this->_allocUniforms();

    // create the descriptor set for the uniform buffer
    this->_initDescriptors();

    this->_initRenderPass ();
    this->_initPipeline ();

    // create framebuffers for the swap chain
    this->_swap.initFramebuffers (this->_renderPass);

    // set up the command buffer
    this->_cmdBuf = this->_app->newCommandBuf();

    // enable handling of keyboard events
    this->enableKeyEvent (true);
}

Lab4Window::~Lab4Window ()
{
    auto device = this->device();

    /* delete the command buffer */
    this->_app->freeCommandBuf(this->_cmdBuf);

    device.destroyPipeline(this->_graphicsPipeline);
    device.destroyPipelineLayout(this->_pipelineLayout);
    device.destroyRenderPass(this->_renderPass);

    device.destroyDescriptorPool(this->_descPool);
    device.destroyDescriptorSetLayout(this->_descSetLayout);
    device.destroySampler(this->_txtSampler);

    delete this->_ubo;
    delete this->_idxBuffer;
    delete this->_vertBuffer;
    delete this->_txt;

}

void Lab4Window::_initDescriptors ()
{
    auto device = this->device();

    // create the descriptor pool
    std::array<vk::DescriptorPoolSize,2> poolSizes = {
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
        };
    vk::DescriptorPoolCreateInfo poolInfo(
        {}, /* flags */
        1, /* max sets */
        poolSizes); /* pool sizes */
    this->_descPool = device.createDescriptorPool(poolInfo);

    // two layouts; one for the UBO and one for the sampler
    std::array<vk::DescriptorSetLayoutBinding, 2> layoutBindings = {
            // the descriptor set layout for the uniform buffer
            vk::DescriptorSetLayoutBinding(
                0, /* binding */
                vk::DescriptorType::eUniformBuffer, /* descriptor type */
                1, /* descriptor count */
                vk::ShaderStageFlagBits::eVertex, /* stages */
                nullptr), /* samplers */
            // the descriptor set layout for the sampler
/** HINT: define the layout bindings for the sampler here */
            vk::DescriptorSetLayoutBinding(
                1, /* binding */
                vk::DescriptorType::eCombinedImageSampler, /* descriptor type */
                1, /* descriptor count */
                vk::ShaderStageFlagBits::eFragment, /* stages */
                nullptr) /* samplers */
        };
    vk::DescriptorSetLayoutCreateInfo layoutInfo(
        {}, /* flags */
        layoutBindings); /* bindings */
    this->_descSetLayout = device.createDescriptorSetLayout(layoutInfo);

    // create the descriptor set
    vk::DescriptorSetAllocateInfo allocInfo(
        this->_descPool,
        this->_descSetLayout);
    this->_descSet = (device.allocateDescriptorSets(allocInfo))[0];

    // info about the UBO
    auto bufferInfo = this->_ubo->descInfo();
    /** HINT: define the info for the sampler here */
    std::array<vk::WriteDescriptorSet,2> descriptorWrites = {
            vk::WriteDescriptorSet(
                this->_descSet, /* descriptor set */
                0, /* binding */
                0, /* array element */
                vk::DescriptorType::eUniformBuffer, /* descriptor type */
                nullptr, /* image info */
                bufferInfo, /* buffer info */
                nullptr), /* texel buffer view */
            /** HINT: define the layout bindings for the sampler here */
        };

    device.updateDescriptorSets (descriptorWrites, nullptr);

}

void Lab4Window::_initRenderPass ()
{
    // we have both color and depth-buffer attachments
    std::vector<vk::AttachmentDescription> atDescs;
    std::vector<vk::AttachmentReference> atRefs;
    this->_initAttachments (atDescs, atRefs);
    assert (atRefs.size() == 2);  /* expect a depth buffer */

    // subpass for output
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

void Lab4Window::_initPipeline ()
{
    // load the shaders
    vk::ShaderStageFlags stages =
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    auto shaders = new cs237::Shaders(this->device(), kShaderDir + "shader", stages);

    // vertex input info
    auto vertexInfo = cs237::vertexInputInfo (
        Vertex::getBindingDescriptions(),
        Vertex::getAttributeDescriptions());

    this->_pipelineLayout = this->_app->createPipelineLayout(this->_descSetLayout);

    std::array<vk::DynamicState, 2> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    this->_graphicsPipeline = this->_app->createPipeline(
        shaders,
        vertexInfo,
        vk::PrimitiveTopology::eTriangleList,
        // the viewport and scissor rectangles are specified dynamically, but we need
        // to specify the counts
        vk::ArrayProxy<vk::Viewport>(1, nullptr), /* viewports */
        vk::ArrayProxy<vk::Rect2D>(1, nullptr), /* scissor rects */
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        // we are following the OpenGL convention for front faces
        vk::FrontFace::eCounterClockwise,
        this->_pipelineLayout,
        this->_renderPass,
        0,
        dynamicStates);

    cs237::destroyVertexInputInfo (vertexInfo);
    delete shaders;

}

void Lab4Window::_initData ()
{
    Mesh mesh;

    // create and set up the vertex buffer
    /** HINT: create and initialize this->_vertBuffer */

    // create and set up the index buffer
    /** HINT: create and initialize this->_idxBuffer */

    // initialize the texture
    /** HINT: create a texture from `mesh.image` */

}

void Lab4Window::_allocUniforms ()
{
    // create and set up the uniform buffer
    this->_ubo = new cs237::UniformBuffer<UB>(this->_app);

    // set the initial uniform values
    this->_initUniforms ();

}

void Lab4Window::_recordCommandBuffer (uint32_t imageIdx)
{
    vk::CommandBufferBeginInfo beginInfo;
    this->_cmdBuf.begin(beginInfo);

    std::array<vk::ClearValue,2> clearValues = {
            vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f), /* clear the window to black */
            vk::ClearDepthStencilValue(1.0f, 0.0f)
        };
    vk::RenderPassBeginInfo renderPassInfo(
        this->_renderPass,
        this->_swap.fBufs[imageIdx],
        { {0, 0}, this->_swap.extent }, /* render area */
        clearValues);

    this->_cmdBuf.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    /*** BEGIN COMMANDS ***/
    this->_cmdBuf.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        this->_graphicsPipeline);

    // set the viewport using the OpenGL convention
    this->_setViewportCmd (this->_cmdBuf, true);

    // bind the vertex buffer
    vk::Buffer vertBuffers[] = {this->_vertBuffer->vkBuffer()};
    vk::DeviceSize offsets[] = {0};
    this->_cmdBuf.bindVertexBuffers(0, vertBuffers, offsets);

    // bind the index buffer
    this->_cmdBuf.bindIndexBuffer(
        this->_idxBuffer->vkBuffer(), 0, vk::IndexType::eUint16);

    // bind the descriptor sets
    this->_cmdBuf.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        this->_pipelineLayout,
        0,
        this->_descSet,
        nullptr);

    this->_cmdBuf.drawIndexed(this->_idxBuffer->nIndices(), 1, 0, 0, 0);
    /*** END COMMANDS ***/

    this->_cmdBuf.endRenderPass();

    this->_cmdBuf.end();

}

void Lab4Window::_initUniforms ()
{
    // compute the values for the buffer
    UB ub = {
          // the model-view: MV = V*M = V*I = V
            glm::lookAt(this->_camPos, this->_camAt, this->_camUp),
          // the projection matrix
            glm::perspectiveFov(
                glm::radians(kFOV),
                float(this->_wid), float(this->_ht),
                kNearZ, kFarZ)
        };

    this->_ubo->copyTo(ub);

}

void Lab4Window::reshape (int wid, int ht)
{
    // invoke the super-method reshape method
    this->cs237::Window::reshape(wid, ht);

    // recreate the new framebuffers
    this->_swap.initFramebuffers (this->_renderPass);
}

void Lab4Window::draw ()
{
    // next buffer from the swap chain
    auto imageIndex = this->_syncObjs.acquireNextImage ();
    if (imageIndex.result != vk::Result::eSuccess) {
        ERROR("inable to acquire next image");
    }
    int idx = imageIndex.value;

    this->_syncObjs.reset();

    this->_cmdBuf.reset();
    this->_recordCommandBuffer (idx);

    // set up submission for the graphics queue
    this->_syncObjs.submitCommands (this->graphicsQ(), this->_cmdBuf);

    // set up submission for the presentation queue
    this->_syncObjs.present (this->presentationQ(), idx);
}

void Lab4Window::key (int key, int scancode, int action, int mods)
{
  // ignore releases, control keys, command keys, etc.
    if ((action == GLFW_PRESS)
    && (mods & (GLFW_MOD_CONTROL|GLFW_MOD_ALT|GLFW_MOD_SUPER)) == 0) {
        switch (key) {
        case GLFW_KEY_Q:  // 'q' or 'Q' ==> quit
            glfwSetWindowShouldClose (this->_win, true);
            break;

        /** HINT: handle keys for camera control */

        default: // ignore all other keys
            return;
        }
    }

}

/******************** Lab4 class ********************/

Lab4::Lab4 (std::vector<std::string> const &args)
  : cs237::Application (args, "CS237 Lab 4"), _win(nullptr)
{ }

Lab4::~Lab4 ()
{
    if (this->_win != nullptr) { delete this->_win; }
}

void Lab4::run ()
{
    this->_win = new Lab4Window (this);

    // wait until the window is closed
    while(! this->_win->windowShouldClose()) {
        this->_win->draw ();
        glfwWaitEvents();
    }

    // wait until any in-flight rendering is complete
    vkDeviceWaitIdle(this->_device);
}

/******************** main ********************/

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv+argc);
    Lab4 app(args);

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
