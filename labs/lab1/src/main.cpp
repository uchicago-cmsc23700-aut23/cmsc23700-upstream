/*! \file main.cpp
 *
 * CMSC 23700 Autumn 2023 Lab 1.  This file is the main program
 * for Lab1.
 *
 * The lab is derived from the Vulkan tutorial that can be found at
 *
 *      https://vulkan-tutorial.com
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hpp"

#ifdef CS237_BINARY_DIR
const std::string kShaderDir = CS237_BINARY_DIR "/labs/lab1/shaders/";
#else
# error CS237_BINARY_DIR not defined
#endif

//! 2D vertices with color
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindings(1);
        bindings[0].binding = 0;
        bindings[0].stride = sizeof(Vertex);
        bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindings;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attrs(2);

        // pos
        attrs[0].binding = 0;
        attrs[0].location = 0;
        attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
        attrs[0].offset = offsetof(Vertex, pos);

        // color
        attrs[1].binding = 0;
        attrs[1].location = 1;
        attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[1].offset = offsetof(Vertex, color);

        return attrs;
    }
};

//! A 2D triangle to draw
const std::vector<Vertex> vertices = {
        {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}
    };

/******************** derived classes ********************/

//! The Lab 1 Application class
class Lab1 : public cs237::Application {
public:
    Lab1 (std::vector<const char *> &args);
    ~Lab1 ();

    void run () override;
};

//! The Lab 1 Window class
class Lab1Window : public cs237::Window {
public:
    Lab1Window (Lab1 *app);

    ~Lab1Window () override;

    void draw () override;

private:
    VkRenderPass _renderPass;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _graphicsPipeline;
    std::vector<VkFramebuffer> _framebuffers;
    VkCommandPool _cmdPool;
    VkCommandBuffer _cmdBuffer;
    VkBuffer _vertBuffer;
    VkDeviceMemory _vertBufferMemory;
    VkSemaphore _imageAvailable;
    VkSemaphore _renderFinished;
    VkFence _inFlight;

    //! initialize the `_renderPass` field
    void _initRenderPass ();
    //! initialize the `_pipelineLayout` and `_graphicsPipeline` fields
    void _initPipeline ();
    //! initialize the vertex buffer
    void _initVertexBuffer ();
    //! record the rendering commands
    void _recordCommandBuffer (uint32_t imageIdx);

};

/******************** Lab1Window methods ********************/

Lab1Window::Lab1Window (Lab1 *app)
    : cs237::Window (app, cs237::CreateWindowInfo(800, 600))
{
    this->_initRenderPass ();
    this->_initPipeline ();
}

Lab1Window::~Lab1Window ()
{
}

void Lab1Window::_initRenderPass ()
{
}

void Lab1Window::_initPipeline ()
{
}

void Lab1Window::_initVertexBuffer ()
{
}

void Lab1Window::_recordCommandBuffer (uint32_t imageIdx)
{
}

void Lab1Window::draw ()
{
}

/******************** Lab1 class ********************/

Lab1::Lab1 (std::vector<const char *> &args)
  : cs237::Application (args, "CS237 Lab 1")
{ }

Lab1::~Lab1 () { }

void Lab1::run ()
{
}

/******************** main ********************/

int main(int argc, char *argv[])
{
    return EXIT_SUCCESS;
}
