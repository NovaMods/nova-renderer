/*!
 * \author ddubois 
 * \date 03-Dec-17.
 */

#ifndef RENDERER_DESCRIPTOR_SET_MANAGER_H
#define RENDERER_DESCRIPTOR_SET_MANAGER_H

#include <vulkan/vulkan.hpp>
#include "../renderpasses/materials.h"
#include "../../vulkan/render_context.h"
#include "../shaders/auto_allocated_buffer.h"
#include "../shaders/pipeline.h"
#include "texture_manager.h"
#include "uniform_buffer_store.h"

namespace nova {
    /*!
     * \brief Creates and provides access to many of the resources that a shader might need
     */
    class shader_resource_manager {
    public:
        explicit shader_resource_manager(std::shared_ptr<render_context> context);

        ~shader_resource_manager();

        std::shared_ptr<auto_buffer> get_per_model_buffer();

        /*
         * DESCRIPTOR SETS
         */

        void create_descriptor_pool(uint32_t num_sets, uint32_t num_buffers, uint32_t num_textures);

        void create_descriptor_sets(std::unordered_map<std::string, std::vector<pipeline_object>>& pipelines);

        void create_descriptor_sets_for_pipeline(pipeline_object& pipeline_data);

        /*
         * GETTERS
         */

        vk::Sampler get_point_sampler();

        texture_manager& get_texture_manager();

        uniform_buffer_store& get_uniform_buffers();

    private:
        vk::Device device;

        vk::DescriptorPool descriptor_pool;

        std::shared_ptr<auto_buffer> per_model_resources_buffer;

        texture_manager textures;

        uniform_buffer_store ubo_manager;

        /*
         * SAMPLERS
         */

        void create_point_sampler();

        void create_bilinear_sampler();

        void create_aniso_filter();


        std::shared_ptr<render_context> context;
        vk::Sampler point_sampler;
    };
}


#endif //RENDERER_DESCRIPTOR_SET_MANAGER_H
