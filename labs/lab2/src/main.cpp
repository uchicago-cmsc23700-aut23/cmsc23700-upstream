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
    ~Lab2 ();

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
}

Lab2Window::~Lab2Window ()
{
}

void Lab2Window::_initRenderPass ()
{
}

void Lab2Window::_initPipeline ()
{
}

void Lab2Window::_initVertexBuffer ()
{
}

void Lab2Window::_recordCommandBuffer (uint32_t imageIdx)
{
}

void Lab2Window::draw ()
{
}

void Lab2Window::key (int key, int scancode, int action, int mods)
{
}

/******************** Lab2 class ********************/

Lab2::Lab2 (std::vector<std::string> const &args)
  : cs237::Application (args, "CS237 Lab 1")
{ }

Lab2::~Lab2 () { }

void Lab2::run ()
{
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
