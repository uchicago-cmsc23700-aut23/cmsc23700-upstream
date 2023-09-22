/*! \file cs237-texture.hpp
 *
 * Support code for CMSC 23700 Autumn 2023.  A wrapper around the
 * Vulkan image and device memory used to represent textures.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _CS237_TEXTURE_HPP_
#define _CS237_TEXTURE_HPP_

namespace cs237 {

namespace __detail {

class TextureBase {
public:
    //! return the image view for the texture
    vk::ImageView view () const { return this->_view; }

protected:
    Application *_app;          //!< the owning application
    vk::Image _img;             //!< Vulkan image to hold the texture
    vk::DeviceMemory _mem;      //!< device memory for the texture image
    vk::ImageView _view;        //!< image view for texture image

    TextureBase (
        Application *app,
        uint32_t wid, uint32_t ht,
        cs237::__detail::ImageBase const *img);
    ~TextureBase ();

    //! \brief create a vk::Buffer object
    //! \param size   the size of the buffer in bytes
    //! \param usage  the usage of the buffer
    //! \return the allocated buffer
    vk::Buffer _createBuffer (size_t size, vk::BufferUsageFlags usage)
    {
        return this->_app->_createBuffer (size, usage);
    }

    //! \brief A helper function for allocating and binding device memory for a buffer
    //! \param buf    the buffer to allocate memory for
    //! \param props  requred memory properties
    //! \return the device memory that has been bound to the buffer
    vk::DeviceMemory _allocBufferMemory (vk::Buffer buf, vk::MemoryPropertyFlags props)
    {
        return this->_app->_allocBufferMemory (buf, props);
    }

};

} // namespace __detail

// 1D Textures
class Texture1D : public __detail::TextureBase {
public:

    //! \brief Construct a 1D texture from a 1D image
    //! \param app  the owning application
    //! \param img  the source image for the texture
    Texture1D (Application *app, Image1D const *img);

};

// 2D Textures
class Texture2D : public __detail::TextureBase {
public:

    //! \brief Construct a 2D texture from a 2D image
    //! \param app     the owning application
    //! \param img     the source image for the texture
    //! \param mipmap  if true, generate mipmap levels for the texture.
    Texture2D (Application *app, Image2D const *img, bool mipmap = false);
};

} // namespace cs237

#endif // !_CS237_TEXTURE_HPP_
