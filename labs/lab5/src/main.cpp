/*! \file main.cpp
 *
 * CMSC 23700 Autumn 2023 Lab 5.  The main program for Lab 5.
 *
 * \author John Reppy
 */

/*
 * COPYRIGHT (c) 2023 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hpp"
#include "mesh.hpp"
#include "drawable.hpp"
#include "uniforms.hpp"

#ifdef CS237_BINARY_DIR
///< the absolute path to the directory containing the compiled shaders
const std::string kShaderDir = CS237_BINARY_DIR "/labs/lab5/shaders/";
#else
# error CS237_BINARY_DIR not defined
#endif

// view parameters; these are constants for now.
constexpr float kNearZ = 0.2f;       ///< distance to near plane
constexpr float kFarZ = 100.0f;      ///< distance to far plane
constexpr float kFOV = 90.0f;        ///< field of view angle in degrees
constexpr float kRadius = 10.0f;     ///< camera distance from Y axis
constexpr float kCamPosY = 8.0f;     ///< camera elevation
constexpr float kCameraSpeed = 2.0f; ///< camera rotation speed (in degrees)

/// the light's direction in world coordinates (pointing toward the scene)
constexpr glm::vec3 kLightDir(-0.75f, -1.0f, -0.5f);
/// the distance to the light's near plane
constexpr float kLightNearZ = 0.2f;

/// the dimensions of the depth texture
constexpr uint32_t kDepthTextureWid = 1024;
constexpr uint32_t kDepthTextureHt = 1024;

/******************** derived classes ********************/

/// The Lab 5 Application class
class Lab5 : public cs237::Application {
public:
    Lab5 (std::vector<std::string> const &args);
    ~Lab5 ();

    void run () override;

private:
    class Lab5Window *_win;
};

/// The Lab 5 Window class
class Lab5Window : public cs237::Window {
public:
    Lab5Window (Lab5 *app);

    ~Lab5Window () override;

    /// reshape the window
    void reshape (int wid, int ht) override;

    void draw () override;

    /// handle keyboard events
    void key (int key, int scancode, int action, int mods) override;

private:
    // depth (aka shadow) rendering pass
    cs237::DepthBuffer *_depthBuf;              ///< depth-buffer
    vk::RenderPass _depthRenderPass;            ///< render pass for depth texture
    vk::PipelineLayout _depthPipelineLayout;
    vk::Pipeline _depthPipeline;
    vk::Framebuffer _depthFramebuffer;

    // view rendering pass
    vk::RenderPass _viewRenderPass;
    vk::PipelineLayout _viewPipelineLayout;
    vk::Pipeline _viewPipeline;
    std::vector<vk::Framebuffer> _framebuffers;

    // descriptors
    vk::DescriptorPool _descPool;               ///< descriptor-set pool
    vk::DescriptorSetLayout _drawableDSLayout;  ///< the descriptor-set layout for the
                                                ///  per-drawable descriptor sets
    vk::DescriptorSetLayout _depthDSLayout;     ///< the descriptor-set layout for the
                                                ///  per-frame depth-buffer descriptor
                                                ///  set (`_depthDS`)
    vk::DescriptorSet _depthDS;                 ///< the depth-buffer descriptor set

    vk::CommandBuffer _cmdBuf;                  ///< command buffer for rendering
    SyncObjs _syncObjs;
    std::vector<Drawable *> _objs;              ///< the drawable objects with their
                                                ///< uniforms, etc.
    cs237::AABBf_t _bbox;                       ///< axis-aligned box that contains
                                                ///  the objects

    // Camera state
    float _angle;                               ///< rotation angle for camera
    glm::vec3 _camPos;                          ///< camera position in world space
    glm::vec3 _camAt;                           ///< camera look-at point in world space
    glm::vec3 _camUp;                           ///< camera up vector in world space

    // cache information for the UBOs
    glm::mat4 _worldToLight;                    ///< world-space to light-space transform
    UB _uboCache;                               ///< cache the per-drawable UBO data
    bool _uboNeedsUpdate;                       ///< flag to mark when the contents of
                                                ///  the per-object UBOs is invalid

    /// initialize the descriptor pool and descriptor-set layouts
    void _initDescriptorSetLayouts ();

    /// initialize the descriptor-sets
    void _initDescriptors ();

    /// initialize the `_depthRenderPass` field
    void _initDepthRenderPass ();
    /// initialize the `_depthPipelineLayout` and `_depthGraphicsPipeline` fields
    void _initDepthPipeline ();

    /// initialize the `_viewRenderPass` field
    void _initViewRenderPass ();
    /// initialize the `_pipelineLayout` and `_viewPipeline` fields
    void _initViewPipeline ();

    /// allocate and initialize the drawables
    void _initDrawables ();

    /// initialize the shadow matrix
    void _initShadowMatrix ();

    /// record the rendering commands
    void _recordCommandBuffer (uint32_t imageIdx);

    /// set the camera position based on the current angle
    void _setCameraPos ()
    {
        float rAngle = glm::radians(this->_angle);
        float camX = kRadius * sin(rAngle);
        float camZ = kRadius * cos(rAngle);
        this->_camPos = glm::vec3(camX, kCamPosY, camZ);

        // update the UBO cache
        this->_uboCache.viewMat = glm::lookAt(this->_camPos, this->_camAt, this->_camUp);
        this->_uboNeedsUpdate = true;
    }

    /// set the projection matrix based on the current window size
    void _setProjMat ()
    {
        this->_uboCache.projMat = glm::perspectiveFov(
            glm::radians(kFOV),
            float(this->_wid), float(this->_ht),
            kNearZ, kFarZ);
        this->_uboNeedsUpdate = true;
    }

};

/******************** Lab5Window methods ********************/

Lab5Window::Lab5Window (Lab5 *app)
  : cs237::Window (
        app,
        // resizable window with depth buffer and no stencil
        cs237::CreateWindowInfo(1024, 768, app->name(), true, true, false)),
    _syncObjs(this)
{
    // initialize the camera
    this->_angle = 0.0;
    this->_camAt = glm::vec3(0.0f, 0.0f, 0.0f);
    this->_camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    this->_setCameraPos ();

    // initialize the projection matrix
    this->_setProjMat();

    // cache the unit vector that points toward the light
    this->_uboCache.lightDir = glm::normalize(-kLightDir);

    // initially, texturing and shadowing are disabled
    this->_uboCache.enableTexture = VK_FALSE;
    this->_uboCache.enableShadows = VK_FALSE;

    this->_initDrawables ();

    this->_initShadowMatrix();

    // initialize the UBOs for the objects
    for (auto obj : this->_objs) {
        obj->updateUBO(this->_worldToLight, this->_uboCache);
    }

    this->_initDescriptorSetLayouts ();

    // create the depth buffer; note that this step needs to be
    // done before calling _initDepthRenderPass, since we need
    // to determine the format of the depth buffer
    this->_depthBuf = new cs237::DepthBuffer (app, kDepthTextureWid, kDepthTextureHt);

    this->_initDepthRenderPass ();
    this->_initViewRenderPass ();

    this->_initDepthPipeline ();
    this->_initViewPipeline ();

    this->_initDescriptors();

    // create the depth-buffer framebuffer
    this->_depthFramebuffer = this->_depthBuf->createFramebuffer (this->_depthRenderPass);

    // create framebuffers for the swap chain
    this->_swap.initFramebuffers (this->_viewRenderPass);

    // set up the command buffer
    this->_cmdBuf = this->_app->newCommandBuf();

    // enable handling of keyboard events
    this->enableKeyEvent (true);
}

Lab5Window::~Lab5Window ()
{
    auto device = this->device();

    /* delete the command buffer */
    this->_app->freeCommandBuf(this->_cmdBuf);

    // clean up view resources
    device.destroyPipeline(this->_viewPipeline);
    device.destroyRenderPass(this->_viewRenderPass);
    device.destroyPipelineLayout(this->_viewPipelineLayout);

    // clean up depth-buffer resources
    device.destroyFramebuffer(this->_depthFramebuffer);
    delete this->_depthBuf;
    device.destroyPipeline(this->_depthPipeline);
    device.destroyRenderPass(this->_depthRenderPass);
    device.destroyPipelineLayout(this->_depthPipelineLayout);

    // delete the objects and associated resources
    for (auto obj : this->_objs) {
        delete obj;
    }

    // clean up other resources
    device.destroyDescriptorPool(this->_descPool);
    device.destroyDescriptorSetLayout(this->_drawableDSLayout);
    device.destroyDescriptorSetLayout(this->_depthDSLayout);
}

void Lab5Window::_initDrawables ()
{
    Mesh *floorMesh = Mesh::floor();
    this->_objs.push_back (new Drawable (this->_app, floorMesh));

    Mesh *crateMesh = Mesh::crate();
    this->_objs.push_back (new Drawable (this->_app, crateMesh));

    // compute the bounding box for the scene
    this->_bbox += floorMesh->bbox();
    this->_bbox += crateMesh->bbox();

    // cleanup
    delete crateMesh;
    delete floorMesh;

}

void Lab5Window::_initShadowMatrix ()
{
}

/******************** Descriptor Set Initialization ********************/

void Lab5Window::_initDescriptorSetLayouts ()
{
    int nObjs = this->_objs.size();
    assert (nObjs > 0);

    // allocate the descriptor-set pool.  We have one UBO descriptor and one sampler
    // descriptor per object, plus the depth-buffer sampler
    std::array<vk::DescriptorPoolSize, 2> poolSizes = {
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, nObjs),
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, nObjs+1)
        };
    vk::DescriptorPoolCreateInfo poolInfo(
        {}, /* flags */
        nObjs+1, /* max sets */
        poolSizes); /* pool sizes */
    this->_descPool = this->device().createDescriptorPool(poolInfo);

    // create the descriptor set layout for the drawables, which consists of
    // two bindings: one for the UBO and one for the color-map
    {
        std::array<vk::DescriptorSetLayoutBinding,2> layoutBindings = {
                vk::DescriptorSetLayoutBinding(
                    0, /* binding */
                    vk::DescriptorType::eUniformBuffer, /* descriptor type */
                    1, /* descriptor count */
                    vk::ShaderStageFlagBits::eVertex /* stages */
                        | vk::ShaderStageFlagBits::eFragment,
                    nullptr), /* immutable samplers */
                vk::DescriptorSetLayoutBinding(
                    1, /* binding */
                    vk::DescriptorType::eCombinedImageSampler, /* descriptor type */
                    1, /* descriptor count */
                    vk::ShaderStageFlagBits::eFragment, /* stages */
                    nullptr) /* immutable samplers */
            };

        vk::DescriptorSetLayoutCreateInfo layoutInfo({}, layoutBindings);
        this->_drawableDSLayout = this->device().createDescriptorSetLayout(layoutInfo);
    }

    // create the descriptor-set layout for the depth-buffer sampler
    {
    }
}

void Lab5Window::_initDescriptors ()
{
    // initialize the per-drawable descriptor sets
    for (auto obj : this->_objs) {
        obj->initDescriptors (this->_descPool, this->_drawableDSLayout);
    }

    // create the depth-buffer descriptor set
    vk::DescriptorSetAllocateInfo allocInfo(this->_descPool, this->_depthDSLayout);
    this->_depthDS = (this->device().allocateDescriptorSets(allocInfo))[0];

    // initialize the depth-buffer descriptor set
    vk::DescriptorImageInfo depthImgInfo = this->_depthBuf->imageInfo();
    vk::WriteDescriptorSet depthDescWrite(
        this->_depthDS, /* descriptor set */
        0, /* binding */
        0, /* array element */
        vk::DescriptorType::eCombinedImageSampler, /* descriptor type */
        depthImgInfo, /* image info */
        nullptr, /* buffer info */
        nullptr); /* texel buffer view */
    this->device().updateDescriptorSets(depthDescWrite, nullptr);
}

/******************** Render Pass Initialization ********************/

void Lab5Window::_initDepthRenderPass ()
{
}

void Lab5Window::_initViewRenderPass ()
{
    // we have both color and depth-buffer attachments
    std::vector<vk::AttachmentDescription> atDescs;
    std::vector<vk::AttachmentReference> atRefs;
    this->_initAttachments (atDescs, atRefs);
    assert (atRefs.size() == 2);  /* expect a depth buffer */

    // subpass for output
    vk::SubpassDescription subpass(
        {}, /* flags */
        vk::PipelineBindPoint::eGraphics, /* pipeline bind point */
        0, nullptr, /* input attachments */
        1, &(atRefs[0]), /* color attachments */
        nullptr, /* resolve attachments */
        &(atRefs[1]), /* depth-stencil attachment */
        0, nullptr); /* preserve attachments */

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
        atDescs, /* attachments */
        subpass, /* subpasses_ */
        dependency); /* dependencies */

    this->_viewRenderPass = this->device().createRenderPass(renderPassInfo);

}

/******************** Graphics Pipeline Initialization ********************/

void Lab5Window::_initDepthPipeline ()
{
}

void Lab5Window::_initViewPipeline ()
{
    // initialize the pipeline layout for rendering
    std::array<vk::DescriptorSetLayout,2> layouts = {
            this->_drawableDSLayout,
            this->_depthDSLayout
        };
    vk::PipelineLayoutCreateInfo layoutInfo(
        {}, /* flags */
        layouts, /* set layouts */
        nullptr); /* push constants */
    this->_viewPipelineLayout = this->device().createPipelineLayout(layoutInfo);

    // load the shaders
    vk::ShaderStageFlags stages =
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    auto shaders = new cs237::Shaders(this->device(), kShaderDir + "scene", stages);

    // vertex input info
    auto vertexInfo = cs237::vertexInputInfo (
        Vertex::getBindingDescriptions(),
        Vertex::getAttributeDescriptions());

    std::array<vk::DynamicState, 2> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    this->_viewPipeline = this->_app->createPipeline(
        shaders,
        vertexInfo,
        vk::PrimitiveTopology::eTriangleList,
        // the viewport and scissor rectangles are specified dynamically, but we need
        // to specify the counts
        vk::ArrayProxy<vk::Viewport>(1, nullptr), /* viewports */
        vk::ArrayProxy<vk::Rect2D>(1, nullptr), /* scissor rects */
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        // we are following the OpenGL convention for front faces
        vk::FrontFace::eCounterClockwise,
        this->_viewPipelineLayout,
        this->_viewRenderPass,
        0,
        dynamicStates);

    cs237::destroyVertexInputInfo (vertexInfo);
    delete shaders;

}

/******************** Rendering ********************/

void Lab5Window::_recordCommandBuffer (uint32_t imageIdx)
{
    vk::ClearValue depthClearValue = vk::ClearDepthStencilValue(1.0f, 0);

    vk::CommandBufferBeginInfo beginInfo;
    this->_cmdBuf.begin(beginInfo);

    if (this->_uboCache.enableShadows) {
        /* Shadow pass */
        vk::RenderPassBeginInfo depthPassInfo(
            this->_depthRenderPass,
            this->_depthFramebuffer,
            vk::Rect2D({0, 0}, {kDepthTextureWid, kDepthTextureHt}),
            depthClearValue);
        this->_cmdBuf.beginRenderPass(depthPassInfo, vk::SubpassContents::eInline);

        /*** BEGIN COMMANDS ***/
        this->_cmdBuf.bindPipeline(
            vk::PipelineBindPoint::eGraphics,
            this->_depthPipeline);

        // draw the objects
        for (auto obj : this->_objs) {
            // bind the descriptor sets for the ubo and color-map samplers
            obj->bindDescriptorSets(this->_cmdBuf, this->_depthPipelineLayout);
            // render the drawable to the shadow buffer
            obj->draw (this->_cmdBuf);
        }
        /*** END COMMANDS ***/

        this->_cmdBuf.endRenderPass();
    }

    /* Render pass */
    std::array<vk::ClearValue,2> clearValues = {
            vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f), /* clear the window to black */
            depthClearValue
        };
    vk::RenderPassBeginInfo renderPassInfo(
        this->_viewRenderPass,
        this->_swap.fBufs[imageIdx],
        { {0, 0}, this->_swap.extent }, /* render area */
        clearValues);
    this->_cmdBuf.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    /*** BEGIN COMMANDS ***/
    this->_cmdBuf.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        this->_viewPipeline);

    // set the viewport using the OpenGL convention
    this->_setViewportCmd (this->_cmdBuf, true);

    // bind the descriptor for the depth buffer as Set 1
    this->_cmdBuf.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        this->_viewPipelineLayout,
        1, /* first set */
        this->_depthDS,
        nullptr);

    for (auto obj : this->_objs) {
        // bind the descriptor sets for the ubo and color-map samplers
        obj->bindDescriptorSets(this->_cmdBuf, this->_viewPipelineLayout);

        // render the drawable
        obj->draw (this->_cmdBuf);
    }
    /*** END COMMANDS ***/

    this->_cmdBuf.endRenderPass();

    this->_cmdBuf.end();

}

void Lab5Window::draw ()
{
    // get the next buffer from the swap chain
    auto imageIndex = this->_syncObjs.acquireNextImage ();
    if (imageIndex.result != vk::Result::eSuccess) {
        ERROR("inable to acquire next image");
    }
    int idx = imageIndex.value;

    this->_syncObjs.reset();

    if (this->_uboNeedsUpdate) {
        // first we update the UBOs for the objects
        for (auto obj : this->_objs) {
            obj->updateUBO(this->_worldToLight, this->_uboCache);
        }
        this->_uboNeedsUpdate = false;
    }

    vkResetCommandBuffer(this->_cmdBuf, 0);
    this->_recordCommandBuffer (idx);

    // set up submission for the graphics queue
    this->_syncObjs.submitCommands (this->graphicsQ(), this->_cmdBuf);

    // set up submission for the presentation queue
    this->_syncObjs.present (this->presentationQ(), idx);
}

/******************** User Interaction ********************/

void Lab5Window::reshape (int wid, int ht)
{
    // invoke the super-method reshape method
    this->cs237::Window::reshape(wid, ht);

    // recreate the new framebuffers
    this->_swap.initFramebuffers (this->_viewRenderPass);

    // update the projection matrix
    this->_setProjMat();

}

void Lab5Window::key (int key, int scancode, int action, int mods)
{
  // ignore releases, control keys, command keys, etc.
    if ((action != GLFW_RELEASE)
    || (mods & (GLFW_MOD_CONTROL|GLFW_MOD_ALT|GLFW_MOD_SUPER))) {

        switch (key) {
            case GLFW_KEY_Q:  // 'q' or 'Q' ==> quit
                glfwSetWindowShouldClose (this->_win, true);
                break;

            case GLFW_KEY_T:  // 't' or 'T' ==> toggle texturing
                this->_uboCache.enableTexture =
                    this->_uboCache.enableTexture ? VK_FALSE : VK_TRUE;
                this->_uboNeedsUpdate = true;
                std::cout << "Toggle texturing "
                    << (this->_uboCache.enableTexture ? "on\n" : "off\n");
                break;

            case GLFW_KEY_S:  // 's' or 'S' ==> toggle shadowing
                this->_uboCache.enableShadows =
                    this->_uboCache.enableShadows ? VK_FALSE : VK_TRUE;
                this->_uboNeedsUpdate = true;
                std::cout << "Toggle shadows "
                    << (this->_uboCache.enableShadows ? "on\n" : "off\n");
                break;

            case GLFW_KEY_LEFT:
                this->_angle -= kCameraSpeed;
                this->_setCameraPos();
                break;

            case GLFW_KEY_RIGHT:
                this->_angle += kCameraSpeed;
                this->_setCameraPos();
                break;

            default: // ignore all other keys
                return;
        }
    }

}

/******************** Lab5 class ********************/

Lab5::Lab5 (std::vector<std::string> const &args)
  : cs237::Application (args, "CS237 Lab 5"), _win(nullptr)
{ }

Lab5::~Lab5 ()
{
    if (this->_win != nullptr) { delete this->_win; }
}

void Lab5::run ()
{
    this->_win = new Lab5Window (this);

    // wait until the window is closed
    while(! this->_win->windowShouldClose()) {
        this->_win->draw ();
        glfwWaitEvents();
    }

    // wait until any in-flight rendering is complete
    this->_device.waitIdle();
}

/******************** main ********************/

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv+argc);
    Lab5 app(args);

    // print information about the keyboard commands
    std::cout
        << "# Lab 5 User Interface\n"
        << "#  't' to toggle textures\n"
        << "#  's' to toggle shadows\n"
        << "#  'q' to quit\n"
        << "#  left and right arrow keys to rotate view\n";

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
