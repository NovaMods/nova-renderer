/*!
 * \author ddubois
 * \date 28-Apr-18.
 */

#ifndef NOVA_RENDERER_FRAMEBUFFER_MANAGER_H
#define NOVA_RENDERER_FRAMEBUFFER_MANAGER_H

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

#include "nova_renderer/swapchain.hpp"
#include "nova_renderer/util/utils.hpp"

namespace nova::renderer::rhi {
    struct Fence;
    struct Framebuffer;
    struct Image;

    class VulkanRenderEngine;

    NOVA_EXCEPTION(swapchain_creation_failed);
    NOVA_EXCEPTION(present_failed);

    /*!
     * \brief Deals with the swapchain, yo
     *
     * Methods to get he next swapchain image and whatnot are found here
     *
     * You can even get the framebuffer constructed from the current swapchain. Wow!
     */
    class VulkanSwapchain final : Swapchain {
    public:
        VulkanSwapchain(uint32_t num_swapchain_images,
                        VulkanRenderEngine& render_engine,
                        glm::uvec2 window_dimensions,
                        const std::vector<VkPresentModeKHR>& present_modes);

#pragma region Swapchain implementation
        uint32_t acquire_next_swapchain_image() override;

        void present(uint32_t image_idx) override;
#pragma endregion
        
        [[nodiscard]] VkImageLayout get_layout(uint32_t frame_idx);
        [[nodiscard]] VkExtent2D get_swapchain_extent() const;
        [[nodiscard]] VkFormat get_swapchain_format() const;

        // I've had a lot of bugs with RAII so here's an explicit cleanup method
        void deinit();

        [[nodiscard]] uint32_t get_num_images() const;

    private:
        VulkanRenderEngine& render_engine;

        VkSwapchainKHR swapchain{};
        VkExtent2D swapchain_extent;
        VkPresentModeKHR present_mode;
        VkFormat swapchain_format;

        std::vector<VkImageView> swapchain_image_views;
        std::vector<VkImageLayout> swapchain_image_layouts;

        uint32_t num_swapchain_images;

#pragma region Initialization
        static VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);

        static VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& modes);

        static VkExtent2D choose_surface_extent(const VkSurfaceCapabilitiesKHR& caps, const glm::ivec2& window_dimensions);

        /*!
         * \brief Creates the VkSwapchain and saves some metadata about it
         *
         * \param requested_num_swapchain_images The number of swapchain images you want in the swapchain. Vulkan may or may not create more
         * than you request
         * \param present_modes The present modes you're wiling to use
         * \param window_dimensions The dimensions of the window you'll be presenting to
         *
         * \post The swapchain is created
         * \post swapchain_format is set to the swapchain's actual format
         * \post present_mode is set to the swapchain's actual present mode
         * \post swapchain_extent is set to the swapchain's actual extent
         */
        void create_swapchain(uint32_t requested_num_swapchain_images,
                              const std::vector<VkPresentModeKHR>& present_modes,
                              const glm::ivec2& window_dimensions);

        /*!
         * \brief Gets the images from the swapchain, so we can create framebuffers and whatnot from them
         *
         * \pre The swapchain exists
         */
        std::vector<VkImage> get_swapchain_images();

        /*!
         * \brief Creates an image view, framebuffer, and fence for a specific swapchain image
         *
         * \param image The swapchain image to create resources for
         * \param renderpass The renderpass returned by create_dummy_renderpass
         * \param swapchain_size The size of the swapchain
         *
         * \note This method will add to swapchain_image_views, swapchain_images, framebuffers, and fences. Its intended use is to be called
         * in a loop over all swapchain images, and never called again. It mostly exists to make the constructor cleaner
         */
        void create_resources_for_frame(VkImage image, VkRenderPass renderpass, const glm::uvec2& swapchain_size);

        /*!
         * \brief Transitions all the provided images into COLOR_ATTACHMENT layout
         */
        void transition_swapchain_images_into_color_attachment_layout(const std::vector<VkImage>& images) const;

        /*!
         * \brief Creates a dummy renderpass that only writes to one image - the swapchain. I need it so I can create framebuffers for the
         * swapchain images
         */
        [[nodiscard]] VkRenderPass create_dummy_renderpass() const;
#pragma endregion
    };
} // namespace nova::renderer::rhi

#endif // NOVA_RENDERER_FRAMEBUFFER_MANAGER_H