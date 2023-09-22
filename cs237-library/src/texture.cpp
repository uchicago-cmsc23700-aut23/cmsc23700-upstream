/*! \file texture.cpp
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

namespace cs237 {

namespace __detail {

TextureBase::TextureBase (
    Application *app,
    uint32_t wid, uint32_t ht,
    cs237::__detail::ImageBase const *img)
  : _app(app)
{
    void *data = img->data();
    size_t nBytes = img->nBytes();
    vk::Format fmt = img->format();

    this->_img = app->_createImage (
        wid, ht, fmt,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
    this->_mem = app->_allocImageMemory(
        this->_img,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    this->_view = app->_createImageView(
        this->_img, fmt,
        vk::ImageAspectFlagBits::eColor);

    // create a staging buffer for copying the image
    vk::Buffer stagingBuf = this->_createBuffer (
        nBytes, vk::BufferUsageFlagBits::eTransferSrc);
    vk::DeviceMemory stagingBufMem = this->_allocBufferMemory(
        stagingBuf,
        vk::MemoryPropertyFlagBits::eHostVisible
            | vk::MemoryPropertyFlagBits::eHostCoherent);

    // copy the image data to the staging buffer
    void* stagingData;
    stagingData = app->_device.mapMemory(stagingBufMem, 0, nBytes, {});
    ::memcpy(stagingData, data, nBytes);
    app->_device.unmapMemory(stagingBufMem);

    app->_transitionImageLayout(
        this->_img, fmt,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal);
    app->_copyBufferToImage(this->_img, stagingBuf, nBytes, wid, ht);
    app->_transitionImageLayout(
        this->_img, fmt,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal);

    // free up the staging buffer
    app->_device.freeMemory(stagingBufMem);
    app->_device.destroyBuffer(stagingBuf);

}

TextureBase::~TextureBase ()
{
    this->_app->_device.destroyImageView(this->_view);
    this->_app->_device.destroyImage(this->_img);
    this->_app->_device.freeMemory(this->_mem);

}

} // namespce __detail

/******************** class Texture1D methods ********************/

Texture1D::Texture1D (Application *app, Image1D const *img)
  : __detail::TextureBase(app, img->width(), 1, img)
{
}

/******************** class Texture2D methods ********************/

Texture2D::Texture2D (Application *app, Image2D const *img, bool mipmap)
  : __detail::TextureBase(app, img->width(), img->height(), img)
{
    if (mipmap) {
        ERROR("mipmap generation not supported yet");
    }
}

} // namespace cs237
