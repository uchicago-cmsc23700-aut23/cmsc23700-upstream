/*! \file cs237-buffer.hpp
 *
 * Support code for CMSC 23700 Autumn 2023.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _CS237_BUFFER_HPP_
#define _CS237_BUFFER_HPP_

#ifndef _CS237_HPP_
#error "cs237-buffer.hpp should not be included directly"
#endif

namespace cs237 {

/// A base class for buffer objects of all kinds
class Buffer {
public:
    /// get the Vulkan buffer object for this buffer
    vk::Buffer vkBuffer () const { return this->_buf; }

    /// get the memory object for this buffer
    const MemoryObj *memory () const { return this->_mem; }

    /// get the memory requirements of this buffer
    vk::MemoryRequirements requirements ()
    {
        vk::MemoryRequirements reqs;
        this->_app->_device.getBufferMemoryRequirements(this->_buf, &reqs);
        return reqs;
    }

protected:
    Application *_app;          ///< the application
    vk::Buffer _buf;            ///< the Vulkan buffer object
    MemoryObj *_mem;            ///< the Vulkan memory object that holds the buffer

    /// constructor
    /// \param app    the owning application object
    /// \param usage  specify the purpose of the buffer object
    /// \param sz     the buffer's size in bytes
    Buffer (Application *app, vk::BufferUsageFlags usage, size_t sz)
      : _app(app)
    {
        vk::BufferCreateInfo info(
            {}, /* flags */
            sz,
            usage,
            vk::SharingMode::eExclusive, /* sharingMode */
            {}); /* queueFamilyIndices */

        this->_buf = app->_device.createBuffer (info);
        this->_mem = new MemoryObj(app, this->requirements());

        // bind the memory object to the buffer
        this->_app->_device.bindBufferMemory(this->_buf, this->_mem->_mem, 0);

    }

    /// destructor
    ~Buffer ()
    {
        delete this->_mem;
        this->_app->_device.destroyBuffer (this->_buf, nullptr);
    }

    /// copy data to a subrange of the device memory object
    /// \param src     address of data to copy
    /// \param offset  offset (in bytes) from the beginning of the buffer
    ///                to copy the data to
    /// \param sz      size in bytes of the data to copy
    void _copyTo (const void *src, size_t offset, size_t sz)
    {
        this->_mem->copyTo(src, offset, sz);
    }

    /// copy data to the device memory object
    /// \param src  address of data to copy
    void _copyTo (const void *src) { this->_mem->copyTo(src); }

};

/// Buffer class for vertex data; the type parameter `V` is the type of an
/// individual vertex.
template <typename V>
class VertexBuffer : public Buffer {
public:

    /// the type of vertices
    using VertexType = V;

    /// constructor
    /// \param app     the owning application object
    /// \param nVerts  the number of vertices in the buffer
    VertexBuffer (Application *app, uint32_t nVerts)
      : Buffer (app, vk::BufferUsageFlagBits::eVertexBuffer, nVerts*sizeof(V))
    { }

    /// copy vertices to the device memory object
    /// \param src  proxy array of vertices
    void copyTo (vk::ArrayProxy<V> const &src)
    {
        assert ((src.size() * sizeof(V) <= this->_mem->size()) && "src is too large");
        this->_copyTo(src.data(), 0, src.size()*sizeof(V));
    }

    /// copy vertices to the device memory object
    /// \param src     proxy array of vertices
    /// \param offset  offset from the beginning of the buffer to copy the data to
    void copyTo (vk::ArrayProxy<V> const &src, uint32_t offset)
    {
        assert (((src.size()+offset) * sizeof(V) <= this->_mem->size())
            && "src is too large");
        this->_copyTo(src.data(), offset*sizeof(V), src.size()*sizeof(V));
    }

};

/// Buffer class for index data; the type parameter `I` is the index type.
template <typename I>
class IndexBuffer : public Buffer {
public:

    /// the type of indices
    using IndexType = I;

    /// constructor
    /// \param app       the owning application object
    /// \param nIndices  the number of indices in the buffer
    IndexBuffer (Application *app, uint32_t nIndices)
      : Buffer (app, vk::BufferUsageFlagBits::eIndexBuffer, nIndices*sizeof(I)),
        _nIndices(nIndices)
    { }

    /// get the number of indices in the buffer
    uint32_t nIndices () const { return this->_nIndices; }

    /// copy indices to the device memory object
    /// \param src  proxy array of indices
    void copyTo (vk::ArrayProxy<I> const &src)
    {
        assert ((src.size() * sizeof(I) <= this->_mem->size()) && "src is too large");
        this->_copyTo(src.data(), 0, src.size()*sizeof(I));
    }

    /// copy vertices to the device memory object
    /// \param src     proxy array of indices
    /// \param offset  offset from the beginning of the buffer to copy the data to
    void copyTo (vk::ArrayProxy<I> const &src, uint32_t offset)
    {
        assert (((src.size()+offset) * sizeof(I) <= this->_mem->size())
            && "src is too large");
        this->_copyTo(src.data(), offset*sizeof(I), src.size()*sizeof(I));
    }

private:
    uint32_t _nIndices;

};

/// Buffer class for uniform data; the type parameter `UB` is the C++
/// struct type of the buffer contents
template <typename UB>
class UniformBuffer : public Buffer {
public:

    /// the type of the buffer's contents
    using BufferType = UB;

    /// constructor
    /// \param app  the owning application object
    UniformBuffer (Application *app)
      : Buffer (app, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(UB))
    { }

    /// copy indices to the device memory object
    /// \param[in] src  the buffer contents to copy to the Vulkan memory buffer
    void copyTo (UB const &src)
    {
        this->_copyTo(&src, 0, sizeof(UB));
    }

};

} // namespace cs237

#endif // !_CS237_BUFFER_HPP_
