#pragma once

#include <nova_renderer/rhi/swapchain.hpp>
#include <rx/core/log.h>

#include "nova_renderer/frame_context.hpp"
#include "nova_renderer/procedural_mesh.hpp"
#include "nova_renderer/renderables.hpp"
#include "nova_renderer/rhi/render_device.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"
#include "nova_renderer/shaderpack_data.hpp"
#include "nova_renderer/util/container_accessor.hpp"

#include "resource_loader.hpp"
#include "rhi/pipeline_create_info.hpp"

namespace nova::renderer {
    RX_LOG("rendergraph", rg_log);

    class DeviceResources;

    namespace renderpack {
        struct RenderPassCreateInfo;
    }

#pragma region Structs for rendering
    template <typename RenderCommandType>
    struct MeshBatch {
        size_t num_vertex_attributes{};
        uint32_t num_indices{};

        rhi::Buffer* vertex_buffer = nullptr;
        rhi::Buffer* index_buffer = nullptr;

        /*!
         * \brief A buffer to hold all the per-draw data
         *
         * For example, a non-animated mesh just needs a mat4 for its model matrix
         *
         * This buffer gets re-written to every frame, since the number of renderables in this mesh batch might have changed. If there's
         * more renderables than the buffer can hold, it gets reallocated from the RHI
         */
        rhi::Buffer* per_renderable_data = nullptr;

        rx::vector<RenderCommandType> commands;
    };

    template <typename RenderCommandType>
    struct ProceduralMeshBatch {
        MapAccessor<MeshId, ProceduralMesh> mesh;

        /*!
         * \brief A buffer to hold all the per-draw data
         *
         * For example, a non-animated mesh just needs a mat4 for its model matrix
         *
         * This buffer gets re-written to every frame, since the number of renderables in this mesh batch might have changed. If there's
         * more renderables than the buffer can hold, it gets reallocated from the RHI
         */
        rhi::Buffer* per_renderable_data = nullptr;

        rx::vector<RenderCommandType> commands;

        ProceduralMeshBatch(rx::map<MeshId, ProceduralMesh>* meshes, const MeshId key) : mesh(meshes, key) {}
    };

    struct MaterialPass {
        rx::vector<MeshBatch<StaticMeshRenderCommand>> static_mesh_draws;
        rx::vector<ProceduralMeshBatch<StaticMeshRenderCommand>> static_procedural_mesh_draws;

        rx::vector<rhi::DescriptorSet*> descriptor_sets;
        const rhi::PipelineInterface* pipeline_interface = nullptr;

        void record(rhi::CommandList& cmds, FrameContext& ctx) const;

        static void record_rendering_static_mesh_batch(const MeshBatch<StaticMeshRenderCommand>& batch,
                                                       rhi::CommandList& cmds,
                                                       FrameContext& ctx);
        static void record_rendering_static_mesh_batch(const ProceduralMeshBatch<StaticMeshRenderCommand>& batch,
                                                       rhi::CommandList& cmds,
                                                       FrameContext& ctx);
    };

    struct Pipeline {
        rhi::Pipeline* pipeline = nullptr;
        rhi::PipelineInterface* pipeline_interface = nullptr;

        void record(rhi::CommandList& cmds, FrameContext& ctx) const;
    };
#pragma endregion

#pragma region Metadata structs
    struct FullMaterialPassName {
        rx::string material_name;
        rx::string pass_name;

        bool operator==(const FullMaterialPassName& other) const;

        rx_size hash() const;
    };

    struct MaterialPassKey {
        rx::string pipeline_name;
        uint32_t material_pass_index;
    };

    struct MaterialPassMetadata {
        renderpack::MaterialPass data;
    };

    struct PipelineMetadata {
        PipelineStateCreateInfo data;

        rx::map<FullMaterialPassName, MaterialPassMetadata> material_metadatas{};
    };

    struct RenderpassMetadata {
        renderpack::RenderPassCreateInfo data;
    };
#pragma endregion

    /*!
     * \brief Renderpass that's ready to be recorded into a command list
     *
     * Renderpass has two virtual methods: `record` and `record_inside_renderpass`. `record` records the renderpass in its entirety, while
     * `record_inside_renderpass` only records the inside of the renderpass, not the work needed to begin or end it. I expect that most
     * subclasses will only want to override `record_inside_renderpass`
     */
    class Renderpass {
    public:
        explicit Renderpass(rx::string name, bool is_builtin = false);

        Renderpass(Renderpass&& old) noexcept = default;
        Renderpass& operator=(Renderpass&& old) noexcept = default;

        Renderpass(const Renderpass& other) = delete;
        Renderpass& operator=(const Renderpass& other) = delete;

        virtual ~Renderpass() = default;

        uint32_t id = 0;
        rx::string name;

        bool is_builtin = false;

        rhi::Renderpass* renderpass = nullptr;
        rhi::Framebuffer* framebuffer = nullptr;

        /*!
         * \brief Names of all the pipelines which are in this renderpass
         */
        rx::vector<rx::string> pipeline_names;

        bool writes_to_backbuffer = false;

        rx::vector<rhi::ResourceBarrier> read_texture_barriers;
        rx::vector<rhi::ResourceBarrier> write_texture_barriers;

        /*!
         * \brief Performs the rendering work of this renderpass
         *
         * Custom renderpasses can override this method to perform custom rendering. However, I recommend that you override
         * `render_renderpass_contents` instead. A typical renderpass will need to issue barriers for the resource it uses, and
         * the default renderpass implementation calls `render_renderpass_contents` after issuing those barriers
         *
         * \param cmds The command list that this renderpass should record all its rendering commands into. You may record secondary command
         * lists in multiple threads and execute them with this command list, if you want
         *
         * \param ctx The context for the current frame. Contains information about the available resources, the current frame, and
         * everything you should need to render. If there's something you need that isn't in the frame context, submit an issue on the Nova
         * GitHub
         */
        virtual void execute(rhi::CommandList& cmds, FrameContext& ctx);

        /*!
         * \brief Returns the framebuffer that this renderpass should render to
         */
        [[nodiscard]] rhi::Framebuffer* get_framebuffer(const FrameContext& ctx) const;

    protected:
        /*!
         * \brief Records all the resource barriers that need to take place before this renderpass renders anything
         *
         * By default `render` calls this method before calling `setup_renderpass`. If you override `render`, you'll need to call
         * this method yourself before using any of this renderpass's resources
         */
        virtual void record_pre_renderpass_barriers(rhi::CommandList& cmds, FrameContext& ctx) const;

        /*!
         * \brief Allows a renderpass to perform work before the recording of the actual renderpass
         *
         * This is useful for e.g. uploading streamed in vertex data
         *
         * The default `render` method calls this after `record_pre_renderpass_barriers` and before `record_renderpass_contents`
         */
        virtual void setup_renderpass(rhi::CommandList& cmds, FrameContext& ctx);

        /*!
         * \brief Renders the contents of this renderpass
         *
         * The default `render` method calls this method after `record_pre_renderpass_barriers` and before
         * `record_post_renderpass_barriers`. Thus, I recommend that you override this method instead of `render` - you'll have fewer things
         * to worry about
         *
         * \param cmds The command list that this renderpass should record all its rendering commands into. You may record secondary command
         * lists in multiple threads and execute them with this command list, if you want
         *
         * \param ctx The context for the current frame. Contains information about the available resources, the current frame, and
         * everything you should need to render. If there's something you need that isn't in the frame context, submit an issue on the Nova
         * GitHub
         */
        virtual void record_renderpass_contents(rhi::CommandList& cmds, FrameContext& ctx);

        /*!
         * \brief Records all the resource barriers that need to take place after this renderpass renders anything
         *
         * By default `render` calls this method after calling `render_renderpass_contents`. If you override `render`, you'll need to call
         * this method yourself near the end of your `render` method
         */
        virtual void record_post_renderpass_barriers(rhi::CommandList& cmds, FrameContext& ctx) const;
    };

    /*!
     * \brief Represents Nova's rendergraph
     *
     * The rendergraph can change a lot over the runtime of Nova. Loading or unloading a renderpack will change the available passes, and
     * the order they're executed in
     */
    class Rendergraph {
    public:
        /*!
         * \brief Constructs a Rendergraph which will allocate its internal memory from the provided allocator, and which will execute on
         * the provided device
         */
        Rendergraph(rx::memory::allocator* allocator, rhi::RenderDevice& device);

        /*!
         * \brief Creates a new renderpass of the specified type using it's own create info
         *
         * This method calls a static method `RenderpassType::get_create_info` to get the renderpass's create info, and it allocates the new
         * renderpass from the rendergraph's internal allocator. Intended usage is adding renderpasses from C++ code - this method makes it
         * easy to define all your renderpass data in your C++ renderpass class
         *
         * This method creates all the GPU resources needed for the renderpass and it's framebuffer. It does not create any pipelines or
         * materials that may be rendered as part of this renderpass. You may create them through the rendergraph's JSON files, or through
         * the renderpass's constructor
         *
         * This method returns a pointer to the newly-created renderpass if everything went according to plan, or `nullptr` if it didn't
         *
         * Exact name and usage are still under revision, this is the alpha version of this method
         */
        template <typename RenderpassType, typename... Args>
        [[nodiscard]] RenderpassType* create_renderpass(DeviceResources& resource_storage, Args&&... args);

        /*!
         * \brief Adds an already-created renderpass with a specific create info
         *
         * This method initializes all the GPU resources needed for this renderpass and the framebuffer it renders to. It then adds the
         * renderpass to the appropriate places, returning a pointer to the renderpass you provided
         *
         * This method returns `nullptr` if the renderpass's GPU resources can't be initialized
         */
        template <typename RenderpassType>
        [[nodiscard]] RenderpassType* add_renderpass(RenderpassType* renderpass,
                                                     const renderpack::RenderPassCreateInfo& create_info,
                                                     DeviceResources& resource_storage);

        void destroy_renderpass(const rx::string& name);

        [[nodiscard]] rx::vector<rx::string> calculate_renderpass_execution_order();

        [[nodiscard]] Renderpass* get_renderpass(const rx::string& name) const;

        [[nodiscard]] rx::optional<RenderpassMetadata> get_metadata_for_renderpass(const rx::string& name) const;

    private:
        bool is_dirty = false;

        rx::memory::allocator* allocator;

        rhi::RenderDevice& device;

        rx::map<rx::string, Renderpass*> renderpasses;

        rx::vector<rx::string> cached_execution_order;
        rx::map<rx::string, RenderpassMetadata> renderpass_metadatas;
    };

    template <typename RenderpassType, typename... Args>
    RenderpassType* Rendergraph::create_renderpass(DeviceResources& resource_storage, Args&&... args) {

        auto* renderpass = allocator->create<RenderpassType>(rx::utility::forward<Args>(args)...);
        const auto& create_info = RenderpassType::get_create_info();

        return add_renderpass(renderpass, create_info, resource_storage);
    }

    template <typename RenderpassType>
    RenderpassType* Rendergraph::add_renderpass(RenderpassType* renderpass,
                                                const renderpack::RenderPassCreateInfo& create_info,
                                                DeviceResources& resource_storage) {
        RenderpassMetadata metadata;
        metadata.data = create_info;

        rx::vector<rhi::Image*> color_attachments;
        color_attachments.reserve(create_info.texture_outputs.size());

        glm::uvec2 framebuffer_size(0);

        const auto num_attachments = create_info.depth_texture ? create_info.texture_outputs.size() + 1 :
                                                                 create_info.texture_outputs.size();
        rx::vector<rx::string> attachment_errors;
        attachment_errors.reserve(num_attachments);

        bool missing_render_targets = false;
        create_info.texture_outputs.each_fwd([&](const renderpack::TextureAttachmentInfo& attachment_info) {
            if(attachment_info.name == BACKBUFFER_NAME) {
                if(create_info.texture_outputs.size() == 1) {
                    renderpass->writes_to_backbuffer = true;
                    renderpass->framebuffer = nullptr; // Will be resolved when rendering

                } else {
                    attachment_errors.push_back(rx::string::format(
                        "Pass %s writes to the backbuffer and %zu other textures, but that's not allowed. If a pass writes to the backbuffer, it can't write to any other textures",
                        create_info.name,
                        create_info.texture_outputs.size() - 1));
                }

                framebuffer_size = device.get_swapchain()->get_size();

            } else {
                const auto render_target_opt = resource_storage.get_render_target(attachment_info.name);
                if(render_target_opt) {
                    const auto& render_target = *render_target_opt;

                    color_attachments.push_back(render_target->image);

                    const glm::uvec2 attachment_size = {render_target->width, render_target->height};
                    if(framebuffer_size.x > 0) {
                        if(attachment_size.x != framebuffer_size.x || attachment_size.y != framebuffer_size.y) {
                            attachment_errors.push_back(rx::string::format(
                                "Attachment %s has a size of %dx%d, but the framebuffer for pass %s has a size of %dx%d - these must match! All attachments of a single renderpass must have the same size",
                                attachment_info.name,
                                attachment_size.x,
                                attachment_size.y,
                                create_info.name,
                                framebuffer_size.x,
                                framebuffer_size.y));
                        }

                    } else {
                        framebuffer_size = attachment_size;
                    }

                } else {
                    rg_log(rx::log::level::k_error, "No render target named %s", attachment_info.name);
                    missing_render_targets = true;
                }
            }
        });

        if(missing_render_targets) {
            return nullptr;
        }

        // Can't combine these if statements and I don't want to `.find` twice
        const auto depth_attachment = [&]() -> rx::optional<rhi::Image*> {
            if(create_info.depth_texture) {
                if(const auto depth_tex = resource_storage.get_render_target(create_info.depth_texture->name); depth_tex) {
                    return (*depth_tex)->image;
                }
            }

            return rx::nullopt;
        }();

        if(!attachment_errors.is_empty()) {
            attachment_errors.each_fwd([&](const rx::string& err) { rg_log(rx::log::level::k_error, "%s", err); });

            rg_log(
                rx::log::level::k_error,
                "Could not create renderpass %s because there were errors in the attachment specification. Look above this message for details",
                create_info.name);
            return nullptr;
        }

        ntl::Result<rhi::Renderpass*> renderpass_result = device.create_renderpass(create_info, framebuffer_size, allocator);
        if(renderpass_result) {
            renderpass->renderpass = renderpass_result.value;

        } else {
            rg_log(rx::log::level::k_error, "Could not create renderpass %s: %s", create_info.name, renderpass_result.error.to_string());
            return nullptr;
        }

        // Backbuffer framebuffers are owned by the swapchain, not the renderpass that writes to them, so if the
        // renderpass writes to the backbuffer then we don't need to create a framebuffer for it
        if(!renderpass->writes_to_backbuffer) {
            renderpass->framebuffer = device.create_framebuffer(renderpass->renderpass,
                                                                color_attachments,
                                                                depth_attachment,
                                                                framebuffer_size,
                                                                allocator);
        }

        renderpass->pipeline_names = create_info.pipeline_names;
        renderpass->id = static_cast<uint32_t>(renderpass_metadatas.size());

        destroy_renderpass(create_info.name);

        renderpasses.insert(create_info.name, renderpass);
        renderpass_metadatas.insert(create_info.name, metadata);

        is_dirty = true;

        return renderpass;
    }
} // namespace nova::renderer
