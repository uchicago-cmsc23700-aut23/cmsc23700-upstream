/*! \file main.cpp
 *
 * CMSC 23700 Autumn 2023 Lab 3.  This file is the main program
 * for Lab3.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hpp"

#ifdef CS237_BINARY_DIR
///< the absolute path to the directory containing the compiled shaders
const std::string kShaderDir = CS237_BINARY_DIR "/labs/lab3/shaders/";
#else
# error CS237_BINARY_DIR not defined
#endif

// view parameters; these are constants for now.
static const float kNearZ = 0.2f;       ///< distance to near plane
static const float kFarZ = 50.0f;       ///< distance to far plane
static const float kFOV = 90.0f;       ///< field of view angle in degrees

// layout of the unform buffer for the vertex shader; we use the `alignas`
// annotations to ensure that the values are correctly aligned.  See
// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout
// for details on the alignment requirements.
//
struct UBO {
    alignas(16) glm::mat4 M;            ///< model transform
    alignas(16) glm::mat4 V;            ///< view transform
    alignas(16) glm::mat4 P;            ///< projection transform
};

/// 3D vertices have position and color
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    /// static method for getting the input-binding description for this class
    static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions()
    {
        std::vector<vk::VertexInputBindingDescription> bindings(1);
        bindings[0].binding = 0;
        bindings[0].stride = sizeof(Vertex);
        bindings[0].inputRate = vk::VertexInputRate::eVertex;

        return bindings;
    }

    /// static method for getting the input-attribute description for this class
    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attrs(2);

        // pos
        attrs[0].binding = 0;
        attrs[0].location = 0;
        attrs[0].format = vk::Format::eR32G32B32Sfloat;
        attrs[0].offset = offsetof(Vertex, pos);

        // color
        attrs[1].binding = 0;
        attrs[1].location = 1;
        attrs[1].format = vk::Format::eR32G32B32Sfloat;
        attrs[1].offset = offsetof(Vertex, color);

        return attrs;
    }

    Vertex (glm::vec3 p, glm::vec3 c) : pos(p), color(c) { }

};

/// the corners of a 2x2x2 cube at the origin
static const std::array<Vertex,8> cubeVertices = {
        Vertex({ -2.0f, -2.0f,  2.0f }, { 0.9f, 0.9f, 0.1f }), // 0
        Vertex({ -2.0f,  2.0f,  2.0f }, { 0.9f, 0.1f, 0.1f }), // 1
        Vertex({  2.0f,  2.0f,  2.0f }, { 0.1f, 0.1f, 0.1f }), // 2
        Vertex({  2.0f, -2.0f,  2.0f }, { 0.1f, 0.9f, 0.1f }), // 3
        Vertex({  2.0f, -2.0f, -2.0f }, { 0.9f, 0.9f, 0.9f }), // 4
        Vertex({  2.0f,  2.0f, -2.0f }, { 0.1f, 0.1f, 0.9f }), // 5
        Vertex({ -2.0f,  2.0f, -2.0f }, { 0.9f, 0.1f, 0.9f }), // 6
        Vertex({ -2.0f, -2.0f, -2.0f }, { 0.1f, 0.9f, 0.9f })  // 7
    };

/// the vertex indices of cube faces; each face consists of two
/// triangles in counter-clockwise vertex order.
static const std::array<uint16_t,36> edgeIndices = {
        2, 1, 0, 0, 3, 2,       // front
        6, 5, 4, 4, 7, 6,       // back
        1, 6, 7, 7, 0, 1,       // left
        5, 2, 3, 3, 4, 5,       // right
        5, 6, 1, 1, 2, 5,       // top
        3, 0, 7, 7, 4, 3        // bottom
    };


/******************** derived classes ********************/

/// The Lab 3 Application class
class Lab3 : public cs237::Application {
public:
    Lab3 (std::vector<std::string> const &args);
    ~Lab3 ();

    void run () override;
};

/// The Lab 3 Window class
class Lab3Window : public cs237::Window {
public:
    Lab3Window (Lab3 *app);

    ~Lab3Window () override;

    /// reshape the window
    void reshape (int wid, int ht) override;

    /// refresh the window
    void draw () override;

    /// handle keyboard events
    void key (int key, int scancode, int action, int mods) override;

private:
    vk::RenderPass _renderPass;
    vk::PipelineLayout _pipelineLayout;
    vk::Pipeline _graphicsPipeline;
    vk::CommandPool _cmdPool;
    vk::CommandBuffer _cmdBuf;
    cs237::VertexBuffer<Vertex> *_vertBuffer;   ///< vertex buffer for cube vertices
    cs237::IndexBuffer<uint16_t> *_idxBuffer;   ///< index buffer for cube indices
    cs237::UniformBuffer<UBO> *_ubo;            ///< uniform buffer for vertex shader
    vk::DescriptorSetLayout _descSetLayout;     ///< descriptor-set layout for uniform buffer
    vk::DescriptorPool _descPool;               ///< descriptor-set pool
    vk::DescriptorSet _descSet;                 ///< descriptor set for uniform buffer
    SyncObjs _syncObjs;
  // Camera state
    glm::vec3 _camPos;                          ///< camera position in world space
    glm::vec3 _camAt;                           ///< camera look-at point in world space
    glm::vec3 _camUp;                           ///< camera up vector in world space

    /// initialize the uniform buffer object using model, view, and projection
    /// matrices that are computed from the current camera state.
    void _initUniforms ();

    /// initialize the UBO descriptors
    void _initDescriptors ();
    /// initialize the `_renderPass` field
    void _initRenderPass ();
    /// initialize the `_pipelineLayout` and `_graphicsPipeline` fields
    void _initPipeline ();
    /// allocate and initialize the buffers
    void _initBuffers ();
    /// record the rendering commands
    void _recordCommandBuffer (uint32_t imageIdx);

};

/******************** Lab3Window methods ********************/

Lab3Window::Lab3Window (Lab3 *app)
  : cs237::Window (
        app,
        // resizable window with depth buffer and no stencil
        cs237::CreateWindowInfo(800, 600, "Lab 3", true, true, false)),
    _syncObjs(this)
{
    // initialize the camera
    this->_camPos = glm::vec3(5.0f, 4.0f, 5.0f);
    this->_camAt = glm::vec3(0.0f, 0.0f, 0.0f);
    this->_camUp = glm::vec3(0.0f, 1.0f, 0.0f);

    this->_initBuffers();

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

Lab3Window::~Lab3Window ()
{
    auto device = this->device();

    /* delete the command buffer */
    this->_app->freeCommandBuf(this->_cmdBuf);

    device.destroyPipeline(this->_graphicsPipeline);
    device.destroyPipelineLayout(this->_pipelineLayout);
    device.destroyRenderPass(this->_renderPass);

    device.destroyDescriptorPool(this->_descPool);
    device.destroyDescriptorSetLayout(this->_descSetLayout);

    delete this->_ubo;
    delete this->_idxBuffer;
    delete this->_vertBuffer;
}

void Lab3Window::_initDescriptors ()
{
    auto device = this->device();

    // create the descriptor pool
    vk::DescriptorPoolSize poolSize(
        vk::DescriptorType::eUniformBuffer,
        1);
    vk::DescriptorPoolCreateInfo poolInfo(
        {}, /* flags */
        1, /* max sets */
        poolSize); /* pool sizes */
    this->_descPool = device.createDescriptorPool(poolInfo);

    // create the descriptor set layout
    vk::DescriptorSetLayoutBinding layoutBinding(
        0, /* binding */
        vk::DescriptorType::eUniformBuffer, /* descriptor type */
        1, /* descriptor count */
        vk::ShaderStageFlagBits::eVertex, /* stages */
        nullptr); /* samplers */
    vk::DescriptorSetLayoutCreateInfo layoutInfo(
        {}, /* flags */
        layoutBinding); /* bindings */
    this->_descSetLayout = device.createDescriptorSetLayout(layoutInfo);

    // create the descriptor set
    vk::DescriptorSetAllocateInfo allocInfo(
        this->_descPool,
        this->_descSetLayout);
    this->_descSet = (device.allocateDescriptorSets(allocInfo))[0];

    // connect the uniform buffer to the descriptor set
    auto bufferInfo = this->_ubo->descInfo();
    vk::WriteDescriptorSet descriptorWrite(
        this->_descSet,
        0, /* binding */
        0, /* array element */
        vk::DescriptorType::eUniformBuffer, /* descriptor type */
        nullptr, /* image info */
        bufferInfo, /* buffer info */
        nullptr); /* texel buffer view */
    device.updateDescriptorSets (descriptorWrite, nullptr);

}

void Lab3Window::_initRenderPass ()
{
    // we have a single output framebuffer as the attachment
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

void Lab3Window::_initPipeline ()
{
    // load the shaders for this lab
    vk::ShaderStageFlags stages =
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    auto shaders = new cs237::Shaders(this->device(), kShaderDir + "shader", stages);

    // vertex input info
    auto vertexInfo = cs237::vertexInputInfo (
        Vertex::getBindingDescriptions(),
        Vertex::getAttributeDescriptions());

    vk::PipelineLayoutCreateInfo layoutInfo(
        {}, /* flags */
        this->_descSetLayout, /* set layouts */
        nullptr); /* push constants */
    this->_pipelineLayout = this->device().createPipelineLayout(layoutInfo);

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

    // cleanup temporaries
    cs237::destroyVertexInputInfo (vertexInfo);
    delete shaders;

}

void Lab3Window::_initBuffers ()
{
    // create the vertex buffer and initialize it with the vertices
    this->_vertBuffer = new cs237::VertexBuffer<Vertex>(this->_app, cubeVertices);

    /** HINT: create and set up the index buffer */

    /** HINT: create and set up the uniform buffer */

    // set the initial uniform values
    this->_initUniforms ();

}

void Lab3Window::_recordCommandBuffer (uint32_t imageIdx)
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

    /** HINT: bind the index buffer */

    /** HINT: bind the descriptor sets */

    this->_cmdBuf.draw(vertices.size(), 1, 0, 0); /** HINT: change this line */
    /*** END COMMANDS ***/

    vkCmdEndRenderPass(this->_cmdBuf);

    if (vkEndCommandBuffer(this->_cmdBuf) != VK_SUCCESS) {
        ERROR("unable to record command buffer!");
    }
}

void Lab3Window::_initUniforms ()
{
    /** HINT: use the GLM lookAt and perspectiveFov functions to compute the
     ** view and projection matrices.
     **/

}

void Lab3Window::draw ()
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

void Lab3Window::reshape (int wid, int ht)
{
    // invoke the super-method reshape method
    this->cs237::Window::reshape(wid, ht);

    // recreate the new framebuffers
    this->_swap.initFramebuffers (this->_renderPass);
}

void Lab3Window::key (int key, int scancode, int action, int mods)
{
  // ignore releases, control keys, command keys, etc.
    if ((action == GLFW_PRESS)
    && (mods & (GLFW_MOD_CONTROL|GLFW_MOD_ALT|GLFW_MOD_SUPER)) == 0) {
        switch (key) {
            case GLFW_KEY_Q:  // 'q' or 'Q' ==> quit
                glfwSetWindowShouldClose (this->_win, true);
                break;

            case GLFW_KEY_UP:
                /** HINT: move the camera's z position forward toward the _camAt point */
                break;

            case GLFW_KEY_DOWN:
                /** HINT: move the camera's z position away from the _camAt point */
                break;

            default: // ignore all other keys
                return;
        }
    }

}

/******************** Lab3 class ********************/

Lab3::Lab3 (std::vector<std::string> const &args)
  : cs237::Application (args, "CS237 Lab 3")
{ }

Lab3::~Lab3 () { }

void Lab3::run ()
{
    Lab3Window *win = new Lab3Window (this);

    // wait until the window is closed
    while(! win->windowShouldClose()) {
        glfwPollEvents();
        win->draw ();
    }

    // wait until any in-flight rendering is complete
    vkDeviceWaitIdle(this->_device);

    // cleanup
    delete win;
}

/******************** main ********************/

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv + argc);
    Lab3 app(args);

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
