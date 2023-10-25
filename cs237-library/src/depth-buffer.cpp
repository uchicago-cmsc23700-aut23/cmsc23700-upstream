/*! \file depth-buffer.cpp
 *
 * Support code for CMSC 23700 Autumn 2023.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hpp"
#include "vulkan/vulkan_core.h"

namespace cs237 {

DepthBuffer::DepthBuffer (Application *app, uint32_t wid, uint32_t ht)
: _app(app), _wid(wid), _ht(ht)
{
    // create the sampler
    cs237::Application::SamplerInfo samplerInfo(
        vk::Filter::eLinear,                    // magnification filter
        vk::Filter::eLinear,                    // minification filter
        vk::SamplerMipmapMode::eNearest,        // mipmap mode
        vk::SamplerAddressMode::eClampToEdge,   // addressing mode for U coordinates
        vk::SamplerAddressMode::eClampToEdge,   // addressing mode for V coordinates
        vk::BorderColor::eFloatOpaqueBlack);    // border color
    this->_sampler = this->_app->createSampler (samplerInfo);

/* TODO: should check device properties to verify that the format works */

/* FIXME: should check to see if we can use eD32Sfloat for the format */
    auto depthFormat = vk::Format::eD16Unorm;

    // create the image
    this->_image = app->_createImage (
        wid, ht,
        depthFormat,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment
            | vk::ImageUsageFlagBits::eSampled,
        1);

    // allocate and bind the memory object
    this->_mem = app->_allocImageMemory (
        this->_image,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    // create the image view
    this->_imageView = app->_createImageView(
        this->_image,
        depthFormat,
        vk::ImageAspectFlagBits::eDepth);

}

DepthBuffer::~DepthBuffer ()
{
    this->_app->device().destroyImageView (this->_imageView);
    this->_app->device().freeMemory (this->_mem);
    this->_app->device().destroyImage (this->_image);
    this->_app->device().destroySampler (this->_sampler);
}

vk::Framebuffer DepthBuffer::createFramebuffer (vk::RenderPass rp)
{
    vk::FramebufferCreateInfo fbInfo(
        {}, /* flags */
        rp, /* render pass */
        this->_imageView, /* attachments */
        this->_wid,
        this->_ht,
        1); /* layers */

    return this->_app->device().createFramebuffer(fbInfo);

}

} // namespace cs237
