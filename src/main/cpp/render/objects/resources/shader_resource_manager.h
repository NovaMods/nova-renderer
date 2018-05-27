/*!
 * \author ddubois 
 * \date 03-Dec-17.
 */

#ifndef RENDERER_DESCRIPTOR_SET_MANAGER_H
#define RENDERER_DESCRIPTOR_SET_MANAGER_H

#include <vulkan/vulkan.hpp>
#include "../renderpasses/materials.h"
#include "../../vulkan/render_context.h"
#include "auto_allocated_buffer.h"
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
		
        /*
         * DESCRIPTOR SETS
         */

        void create_descriptor_pool(uint32_t num_sets, uint32_t num_buffers, uint32_t num_textures);

        void create_descriptor_sets(const std::unordered_map<std::string, std::vector<pipeline_object>>& pipelines,
                                            std::unordered_map<std::string, std::vector<material_pass>>& material_passes);

        std::vector<vk::DescriptorSet> create_descriptor_sets_for_pipeline(const pipeline_object& pipeline_data);

        vk::DescriptorSet create_model_matrix_descriptor();

        void free_descriptor(vk::DescriptorSet to_free);

        /*
         * GETTERS
         */

        vk::Sampler get_point_sampler();

        texture_manager& get_texture_manager();

        uniform_buffer_store& get_uniform_buffers();

        /*
         * Something higher-level
         */

        void update_all_descriptor_sets(const std::unordered_map<std::string, resource_binding>& name_to_descriptor, material_pass& mat);

    private:
        vk::Device device;

        vk::DescriptorPool descriptor_pool;
		
        texture_manager textures;

        uniform_buffer_store buffers;

        std::shared_ptr<render_context> context;
        vk::Sampler point_sampler;
        vk::DescriptorSetLayout model_matrix_layout;
        vk::DescriptorSetAllocateInfo model_matrix_descriptor_allocate_info;

        int per_model_descriptor_count = 0;

        int total_allocated_descriptor_sets = 0;

        /*
         * SAMPLERS
         */

        void create_point_sampler();

        void create_bilinear_sampler();

        void create_aniso_filter();
    };
}


#endif //RENDERER_DESCRIPTOR_SET_MANAGER_H
