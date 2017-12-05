/*!
 * \author ddubois 
 * \date 03-Dec-17.
 */

#ifndef RENDERER_DESCRIPTOR_SET_MANAGER_H
#define RENDERER_DESCRIPTOR_SET_MANAGER_H

#include <vulkan/vulkan.hpp>

namespace nova {
    /*!
     * \brief Creates and provides access to many of the resources that a shader might need
     */
    class shader_resource_manager {
    public:
        shader_resource_manager();

    private:
        vk::Device device;

        vk::DescriptorSetLayout block_textures_dsl;
        vk::DescriptorSetLayout custom_textures_dsl;
        vk::DescriptorSetLayout shadow_textures_dsl;
        vk::DescriptorSetLayout depth_textures_dsl;
        vk::DescriptorSetLayout noise_tex_dsl;
        vk::DescriptorSetLayout framebuffer_top_dsl;
        vk::DescriptorSetLayout framebuffer_bottom_dsl;
        vk::DescriptorSetLayout block_light_dsl;

        vk::DescriptorPool descriptor_pool;

        /*
         * DESCRIPTOR SET LAYOUTS
         */
        void create_block_textures_dsl();

        void create_custom_textures_dsl();

        void create_shadow_textures_dsl();

        void create_depth_textures_dsl();

        void create_noise_tex_dsl();

        void create_framebuffer_top_dsl();

        void create_framebuffer_bottom_dsl();

        void create_block_light_dsl();

        /*
         * DESCRIPTOR SETS
         */

        void create_block_textures_ds();

        /*
         * SAMPLERS
         */

        void create_point_sampler();

        void create_bilinear_sampler();

        void create_aniso_filter();

        void create_descriptor_pool();


    };
}


#endif //RENDERER_DESCRIPTOR_SET_MANAGER_H
