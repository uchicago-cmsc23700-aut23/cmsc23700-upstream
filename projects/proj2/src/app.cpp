/*! \file app.cpp
 *
 * CS23700 Autumn 2023 Sample Code for Project 2
 *
 * The main application class for Project 2.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "app.hpp"
#include "window.hpp"
#include <cstdlib>
#include <unistd.h>

#ifdef CS237_SOURCE_DIR
//!< the absolute path to the directory containing the scenes
const std::string kDataDir = CS237_SOURCE_DIR "/projects/proj2/scenes/";
#else
# error CS237_SOURCE_DIR not defined
#endif

static void usage (int sts)
{
    std::cerr << "usage: proj1 [options] <scene>\n";
    exit (sts);
}

Proj2::Proj2 (std::vector<std::string> const &args)
  : cs237::Application (args, "CS237 Project 2")
{
    // the last argument is the name of the scene directory that we should render
    if (args.size() < 2) {
        usage(EXIT_FAILURE);
    }
    std::string sceneName = args.back();
    std::string scenePath = kDataDir + sceneName;

    // verify that the scene path exists
    if (::access(scenePath.c_str(), F_OK) < 0) {
        std::cerr << "proj1: scene '" << sceneName
            << "' is not accessible or does not exist\n";
        exit(EXIT_FAILURE);
    }

    // load the scene
    if (this->_scene.load(scenePath)) {
        std::cerr << "proj1: cannot load scene from '" << scenePath << "'\n";
        exit(EXIT_FAILURE);
    }
}

Proj2::~Proj2 ()
{
    this->_device.destroyDescriptorSetLayout(this->_meshDSLayout);
    this->_device.destroyDescriptorPool(this->_meshDSPool);
}

void Proj2::run ()
{
    // create the application window
    Proj2Window *win = new Proj2Window (this);

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

vk::DescriptorSet Proj2::allocMeshDS ()
{
    // create the descriptor set
    vk::DescriptorSetAllocateInfo allocInfo(this->_meshDSPool, this->_meshDSLayout);

    return(this->_device.allocateDescriptorSets(allocInfo))[0];

}
