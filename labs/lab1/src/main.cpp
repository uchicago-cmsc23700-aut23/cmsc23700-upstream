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
/// location of the shaders for Lab 1
const std::string kShaderDir = CS237_BINARY_DIR "/labs/lab1/shaders/";
#else
# error CS237_BINARY_DIR not defined
#endif

/******************** derived classes ********************/

/// The Lab 1 Application class
class Lab1 : public cs237::Application {
public:
    /// constructor
    /// \param args  the command-line arguments
    Lab1 (std::vector<std::string> const &args);

    /// destructor
    ~Lab1 ();

    /// run the application code
    void run () override;
};

/// The Lab 1 Window class
class Lab1Window : public cs237::Window {
public:
    /// constructor
    /// \param app  pointer to the application object
    Lab1Window (Lab1 *app);

    /// destructor
    ~Lab1Window () override;

    /// render the contents of the window
    void draw () override;

private:
    vk::RenderPass _renderPass;
    vk::PipelineLayout _pipelineLayout;
    vk::Pipeline _graphicsPipeline;
    std::vector<vk::Framebuffer> _framebuffers;
    vk::CommandPool _cmdPool;
    vk::CommandBuffer _cmdBuf;
    SyncObjs _syncObjs;

    /// initialize the `_renderPass` field
    void _initRenderPass ();
    /// initialize the `_pipelineLayout` and `_graphicsPipeline` fields
    void _initPipeline ();
    /// record the rendering commands
    void _recordCommandBuffer (uint32_t imageIdx);

};

/******************** Lab1Window methods ********************/

Lab1Window::Lab1Window (Lab1 *app)
  : cs237::Window (app, cs237::CreateWindowInfo(800, 600)),
    _syncObjs(this)
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

void Lab1Window::_recordCommandBuffer (uint32_t imageIdx)
{
}

void Lab1Window::draw ()
{
}

/******************** Lab1 class ********************/

Lab1::Lab1 (std::vector<std::string> const &args)
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
