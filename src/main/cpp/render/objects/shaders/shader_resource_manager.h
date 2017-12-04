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

        vk::DescriptorSetLayout main_ds;
        vk::DescriptorSetLayout block_light_ds;

        /*!
         * \brief Creates a DSL that everything except the block light shader uses
         */
        void create_main_dsl();

        /*!
         * \brief Creates a DSL with the shadow cubemap
         */
        void create_block_light_dsl();

        void create_block_textures_ds();

        void create_point_sampler();

        void create_bilinear_sampler();

        void create_aniso_filter();

        void create_descriptor_pool();
    };
}


#endif //RENDERER_DESCRIPTOR_SET_MANAGER_H
