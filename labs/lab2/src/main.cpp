/*! \file main.cpp
 *
 * CMSC 23700 Autumn 2023 Lab 2.  This file is the main program
 * for Lab2.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hpp"

#ifdef CS237_BINARY_DIR
/// location of the shaders for Lab 2
const std::string kShaderDir = CS237_BINARY_DIR "/labs/lab2/shaders/";
#else
# error CS237_BINARY_DIR not defined
#endif

/// 2D vertices with color
struct Vertex {
    glm::vec2 pos;      ///< the vertex position
    glm::vec3 color;    ///< the vertex color

    /// constructor
    /// \param p  the position
    /// \param c  the color
    Vertex (glm::vec2 p, glm::vec3 c) : pos(p), color(c) { }

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
        attrs[0].format = vk::Format::eR32G32Sfloat;
        attrs[0].offset = offsetof(Vertex, pos);

        // color
        attrs[1].binding = 0;
        attrs[1].location = 1;
        attrs[1].format = vk::Format::eR32G32B32Sfloat;
        attrs[1].offset = offsetof(Vertex, color);

        return attrs;
    }
};

/// A 2D triangle to draw
const std::array<Vertex,3> vertices = {
        Vertex({ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}),
        Vertex({ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}),
        Vertex({-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f})
    };

/******************** derived classes ********************/

/// The Lab 2 Application class
class Lab2 : public cs237::Application {
public:
    /// constructor
    /// \param args  the command-line arguments
    Lab2 (std::vector<std::string> const &args);

    /// destructor
    ~Lab2 () override;

    /// run the application code
    void run () override;
};

/// The Lab 2 Window class
class Lab2Window : public cs237::Window {
public:
    /// constructor
    /// \param app  pointer to the application object
    Lab2Window (Lab2 *app);

    /// destructor
    ~Lab2Window () override;

    /// reshape the window
    void reshape (int wid, int ht) override;

    /// render the contents of the window
    void draw () override;

    /// handle keyboard events
    void key (int key, int scancode, int action, int mods) override;

private:
    vk::RenderPass _renderPass;
    vk::PipelineLayout _pipelineLayout;
    vk::Pipeline _graphicsPipeline;
    vk::CommandPool _cmdPool;
    vk::CommandBuffer _cmdBuf;
    cs237::VertexBuffer<Vertex> *_vertBuffer;
    SyncObjs _syncObjs;

    /// initialize the `_renderPass` field
    void _initRenderPass ();
    /// initialize the `_pipelineLayout` and `_graphicsPipeline` fields
    void _initPipeline ();
    /// initialize the vertex buffer
    void _initVertexBuffer ();
    /// record the rendering commands
    void _recordCommandBuffer (uint32_t imageIdx);

};

/******************** Lab2Window methods ********************/

Lab2Window::Lab2Window (Lab2 *app)
  : cs237::Window (app, cs237::CreateWindowInfo(800, 600)),
    _syncObjs(this)
{
    this->_initRenderPass ();
    this->_initPipeline ();

    // create framebuffers for the swap chain
    this->_swap.initFramebuffers (this->_renderPass);

    // set up the command pool
    vk::CommandPoolCreateInfo poolInfo(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        this->_graphicsQIdx());

    this->_cmdPool = this->device().createCommandPool(poolInfo);

    // set up the command buffer
    vk::CommandBufferAllocateInfo allocInfo(
        this->_cmdPool,
        vk::CommandBufferLevel::ePrimary,
        1);

    auto bufs = this->device().allocateCommandBuffers(allocInfo);
    this->_cmdBuf = bufs[0];

    // allocate synchronization objects
    this->_syncObjs.allocate();

}

Lab2Window::~Lab2Window ()
{
    /* delete the command buffer */
    this->device().freeCommandBuffers(this->_cmdPool, this->_cmdBuf);

    /* delete the command pool */
    this->device().destroyCommandPool(this->_cmdPool);

    this->device().destroyPipeline(this->_graphicsPipeline);
    this->device().destroyPipelineLayout(this->_pipelineLayout);
    this->device().destroyRenderPass(this->_renderPass);
}

void Lab2Window::_initRenderPass ()
{
    // we have a single output framebuffer as the attachment
    vk::AttachmentDescription colorAttachment(
        {}, /* flags */
        this->_swap.imageFormat, /* image-view format */
        vk::SampleCountFlagBits::e1, /* number of samples */
        vk::AttachmentLoadOp::eClear, /* load op */
        vk::AttachmentStoreOp::eStore, /* store op */
        vk::AttachmentLoadOp::eDontCare, /* stencil load op */
        vk::AttachmentStoreOp::eDontCare, /* stencil store op */
        vk::ImageLayout::eUndefined, /* initial layout */
        vk::ImageLayout::ePresentSrcKHR); /* final layout */

    vk::AttachmentReference colorAttachmentRef(
        0, /* index */
        vk::ImageLayout::eColorAttachmentOptimal); /* layout */

    vk::SubpassDescription subpass(
        {}, /* flags */
        vk::PipelineBindPoint::eGraphics, /* pipeline bind point */
        {}, /* input attachments */
        colorAttachmentRef, /* color attachments */
        {}, /* resolve attachments */
        {}, /* depth-stencil attachment */
        {}); /* preserve attachments */

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
        colorAttachment, /* attachments */
        subpass, /* subpasses_ */
        dependency); /* dependencies */

    this->_renderPass = this->device().createRenderPass(renderPassInfo);

}

void Lab2Window::_initPipeline ()
{
}

void Lab2Window::_initVertexBuffer ()
{
}

void Lab2Window::_recordCommandBuffer (uint32_t imageIdx)
{
    vk::CommandBufferBeginInfo beginInfo;
    this->_cmdBuf.begin(beginInfo);

    vk::ClearValue blackColor(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));
    vk::RenderPassBeginInfo renderPassInfo(
        this->_renderPass,
        this->_swap.fBufs[imageIdx],
        { {0, 0}, this->_swap.extent }, /* render area */
        blackColor); /* clear the window to black */

    this->_cmdBuf.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    /*** BEGIN COMMANDS ***/
    this->_cmdBuf.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        this->_graphicsPipeline);

    /*** END COMMANDS ***/

    this->_cmdBuf.endRenderPass();

    this->_cmdBuf.end();

}

void Lab2Window::reshape (int wid, int ht)
{
}

void Lab2Window::draw ()
{
    // next buffer from the swap chain
    auto imageIndex = this->_syncObjs.acquireNextImage ();
    if (imageIndex.result != vk::Result::eSuccess) {
        ERROR("inable to acquire next image");
    }

    this->_syncObjs.reset();

    this->_cmdBuf.reset();
    this->_recordCommandBuffer (imageIndex.value);

    // set up submission for the graphics queue
    this->_syncObjs.submitCommands (this->graphicsQ(), this->_cmdBuf);

    // set up submission for the presentation queue
    this->_syncObjs.present (this->presentationQ(), imageIndex.value);
}

void Lab2Window::key (int key, int scancode, int action, int mods)
{
}

/******************** Lab2 class ********************/

Lab2::Lab2 (std::vector<std::string> const &args)
  : cs237::Application (args, "CS237 Lab 2")
{ }

Lab2::~Lab2 () { }

void Lab2::run ()
{
    Lab2Window *win = new Lab2Window (this);

    // wait until the window is closed
    while(! win->windowShouldClose()) {
        glfwPollEvents();
        win->draw ();
    }

    // wait until any in-flight rendering is complete
    this->_device.waitIdle();

    // cleanup
    delete win;
}

/******************** main ********************/

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv + argc);
    Lab2 app(args);

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
