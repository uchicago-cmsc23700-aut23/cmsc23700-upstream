/*! \file cs237-window.hpp
 *
 * Support code for CMSC 23700 Autumn 2023.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _CS237_WINDOW_HPP_
#define _CS237_WINDOW_HPP_

#ifndef _CS237_HPP_
#error "cs237-window.hpp should not be included directly"
#endif

#include <optional>

namespace cs237 {

/// structure containing parameters for creating windows
//
struct CreateWindowInfo {
    int wid;            ///< the window width
    int ht;             ///< the window height
    std::string title;  ///< window title
    bool resizable;     ///< should the window support resizing
    bool depth;         ///< do we need depth-buffer support?
    bool stencil;       ///< do we need stencil-buffer support?

    CreateWindowInfo (int w, int h, std::string const &t, bool r, bool d, bool s)
        : wid(w), ht(h), title(t), resizable(r), depth(d), stencil(s)
    { }
    CreateWindowInfo (int w, int h)
        : wid(w), ht(h), title(""), resizable(false), depth(true), stencil(false)
    { }

    bool needsDepthBuf () const { return this->depth || this->stencil; }
};

/// abstract base class for simple GLFW windows used to view buffers, etc.
//
class Window {
public:

    /// destructor: it destroys the underlying GLFW window
    virtual ~Window ();

    /// return the application pointer
    Application *app () { return this->_app; }

    /// return the logical device for this window
    vk::Device device () const { return this->_app->_device; }

    /// the graphics queue
    vk::Queue graphicsQ () const { return this->_app->_queues.graphics; }

    /// the presentation queue
    vk::Queue presentationQ () const { return this->_app->_queues.present; }

    /// Refresh the contents of the window.  This method is also invoked
    /// on Refresh events.
    void refresh ()
    {
        if (this->_isVis) {
            this->draw();
        }
    }

    /// Hide the window
    void hide ()
    {
      glfwHideWindow (this->_win);
      this->_isVis = false;
    }

    /// Show the window (a no-op if it is already visible)
    void show ()
    {
      glfwShowWindow (this->_win);
      this->_isVis = true;
    }

    /// virtual draw method provided by derived classes to draw the contents of the
    /// window.  It is called by Refresh.
    virtual void draw () = 0;

    /// method invoked on Reshape events.  It resets the viewport and the
    /// projection matrix (see SetProjectionMatrix)
    virtual void reshape (int wid, int ht);

    /// method invoked on Iconify events.
    virtual void iconify (bool iconified);

    /// get the value of the "close" flag for the window
    bool windowShouldClose ()
    {
        return glfwWindowShouldClose (this->_win);
    }

    ///{
    /// Input handling methods; override these in the derived window
    /// classes to do something useful.
    virtual void key (int key, int scancode, int action, int mods);
    virtual void cursorPos (double xpos, double ypos);
    virtual void cursorEnter (bool entered);
    virtual void mouseButton (int button, int action, int mods);
    virtual void scroll (double xoffset, double yoffset);
    //}

    ///{
    /// enable/disable handling of events
    void enableKeyEvent (bool enable);
    void setCursorMode (int mode);
    void enableCursorPosEvent (bool enable);
    void enableCursorEnterEvent (bool enable);
    void enableMouseButtonEvent (bool enable);
    void enableScrollEvent (bool enable);
    //}

protected:
    /// information about swap-chain support
    struct SwapChainDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;

        /// \brief choose a surface format from the available formats
        vk::SurfaceFormatKHR chooseSurfaceFormat ();
        /// \brief choose a presentation mode from the available modes; we prefer
        ///        "mailbox" (aka triple buffering)
        vk::PresentModeKHR choosePresentMode ();
        /// \brief get the extent of the window subject to the limits of the Vulkan device
        vk::Extent2D chooseExtent (GLFWwindow *win);
    };

    struct DepthStencilBuffer {
        vk::Format format;              ///< the depth/image-buffer format
        vk::Image image;                ///< depth/image-buffer image
        vk::DeviceMemory imageMem;      ///< device memory for depth/image-buffer
        vk::ImageView view;             ///< image view for depth/image-buffer
    };

    /// the collected information about the swap-chain for a window
    struct SwapChain {
        vk::Device device;              ///< the owning logical device
        vk::SwapchainKHR chain;         ///< the swap chain object
        vk::Format imageFormat;         ///< pixel format of image buffers
        vk::Extent2D extent;            ///< size of swap buffer images
        int numAttachments;             ///< the number of framebuffer attachments
        // the following vectors hold the state for each of the buffers in the
        // swap chain.
        std::vector<vk::Image> images;  ///< images for the swap buffers
        std::vector<vk::ImageView> views; ///< image views for the swap buffers
        std::optional<DepthStencilBuffer> dsBuf; ///< optional depth/stencil-buffer

        SwapChain (vk::Device dev)
          : device(dev), dsBuf(std::nullopt)
        { }

        /// \brief return the number of buffers in the swap chain
        int size () const { return this->images.size(); }

        /// \brief allocate frame buffers for a rendering pass
        std::vector<vk::Framebuffer> framebuffers (vk::RenderPass renderPass);

        /// \brief destroy the Vulkan state for the swap chain
        void cleanup ();
    };

    /// a container for a frame's synchronization objects
    struct SyncObjs {
        Window *win;                    ///< the owning window
        vk::Semaphore imageAvailable;   ///< semaphore for signaling when the
                                        ///  image object is available
        vk::Semaphore renderFinished;   ///< semaphore for signaling when render pass
                                        ///  is finished
        vk::Fence inFlight;             ///< fence for

        /// \brief create a SyncObjs container
        explicit SyncObjs (Window *w)
          : win(w),
            imageAvailable(VK_NULL_HANDLE),
            renderFinished(VK_NULL_HANDLE),
            inFlight(VK_NULL_HANDLE)
        {
            this->allocate();
        }
        SyncObjs () = delete;
        SyncObjs (SyncObjs &) = delete;
        SyncObjs (SyncObjs const &) = delete;
        SyncObjs (SyncObjs &&) = delete;

        /// destroy the objects
        ~SyncObjs ();

        /// allocate the synchronization objects
        void allocate ();

        /// \brief acquire the next image from the window's swap chain.
        /// \return the next image's index
        vk::ResultValue<uint32_t> acquireNextImage ();

        /// reset the in-flight fence of this frame
        void reset ();

        /// submit a command buffer to a queue using this frame's synchronization objects
        /// \param q        the queue to submit the commands to
        /// \param cmdBuf   the command buffer to submit
        void submitCommands (vk::Queue q, vk::CommandBuffer const &cmdBuf);

        /// \brief present the frame
        /// \param q           the presentation queue
        /// \param imageIndex  the image index to present
        /// \return the return status of presenting the image
        vk::Result present (vk::Queue q, uint32_t imageIndex);
    };

    Application *_app;                  ///< the owning application
    GLFWwindow *_win;                   ///< the underlying window
    int _wid, _ht;	                ///< window dimensions
    bool _isVis;                        ///< true when the window is visible
    bool _keyEnabled;                   ///< true when the Key callback is enabled
    bool _cursorPosEnabled;             ///< true when the CursorPos callback is enabled
    bool _cursorEnterEnabled;           ///< true when the CursorEnter callback is enabled
    bool _mouseButtonEnabled;           ///< true when the MouseButton callback is enabled
    bool _scrollEnabled;                ///< true when the Scroll callback is enabled
    // Vulkan state for rendering
    vk::SurfaceKHR _surf;               ///< the Vulkan surface to render to
    SwapChain _swap;                    ///< buffer-swapping information

    /// \brief the Window base-class constructor
    /// \param app      the owning application
    /// \param info     information for creating the window, such as size and title
    Window (Application *app, CreateWindowInfo const &info);

    /// \brief Get the swap-chain details for a physical device
    SwapChainDetails _getSwapChainDetails ();

    /// \brief Create the swap chain for this window; this initialized the _swap
    ///        instance variable.
    /// \param depth    set to true if requesting depth-buffer support
    /// \param stencil  set to true if requesting stencil-buffer support
    void _createSwapChain (bool depth, bool stencil);

    /// \brief initialize the attachment descriptors and references for the color and
    ///        optional depth/stencil-buffer
    /// \param[out] descs  vector that will contain the attachment descriptors
    /// \param[out] refs   vector that will contain the attachment references
    void _initAttachments (
        std::vector<vk::AttachmentDescription> &descs,
        std::vector<vk::AttachmentReference> &refs);

    /// \brief the graphics queue-family index
    ///
    /// This is a wrapper to allow subclasses access to this information
    uint32_t _graphicsQIdx () const { return this->_app->_qIdxs.graphics; }

    /// \brief the presentation queue
    ///
    /// This is a wrapper to allow subclasses access to this information
    uint32_t _presentationQIdx () const { return this->_app->_qIdxs.present; }

    /// \brief add a command to set the viewport and scissor to the whole window
    ///        using the OpenGL convention of Y increasing up the screen.
    /// \param cmdBuf   the command buffer
    /// \param oglView  if `true` then use the **OpenGL** convention where
    ///                 Y = 0 maps to the bottom of the screen
    ///
    /// **Vulkan** follows the Direct3D convention of using a right-handed NDC
    /// space, which means that Y = 0 maps to the top of the screen, instead
    /// of the bottom (as in OpenGL).  pass `true` as the second argument to
    /// use the **OpenGL** convention.
    void _setViewportCmd (vk::CommandBuffer cmdBuf, bool oglView = false)
    {
        if (oglView) {
            /* NOTE: we negate the height and set the Y origin to ht because Vulkan's
             * viewport coordinates are from top-left down, instead of from
             * bottom-left up. See
             * https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport
             */
            this->_setViewportCmd(cmdBuf,
                0, this->_swap.extent.height,
                this->_swap.extent.width, -this->_swap.extent.height);
        } else {
            this->_setViewportCmd(cmdBuf,
                0, 0,
                this->_swap.extent.width, this->_swap.extent.height);
        }
    }

    /// \brief add a viewport command to the command buffer; this also sets the
    ///        scissor rectangle.
    /// \param cmdBuf   the command buffer
    /// \param x    specifies the Y coordinate of the upper-left corner
    /// \param y    specifies the X coordinate of the upper-left corner
    /// \param wid  specifies the width of the viewport
    /// \param ht   specifies the height of the viewport
    ///
    /// To use the **OpenGL** convention of the Y axis pointing up, specify the
    /// **OpenGL** Y coordinate and a negative height.
    void _setViewportCmd (vk::CommandBuffer cmdBuf,
        int32_t x, int32_t y,
        int32_t wid, int32_t ht);

public:

    /// the width of the window
    int width () const { return this->_swap.extent.width; }

    /// the height of the window
    int height () const { return this->_swap.extent.height; }

};

} // namespace cs237

#endif // !_CS237_WINDOW_HPP_
