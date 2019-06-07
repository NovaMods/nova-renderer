#include "gl3_swapchain.hpp"
#include "gl3_structs.hpp"

namespace nova::renderer::rhi {
    Gl3Swapchain::Gl3Swapchain(const uint32_t num_swapchain_images) : num_frames(num_swapchain_images) {
        for(uint32_t i = 0; i < num_swapchain_images; i++) {
            framebuffers.push_back(new Gl3Framebuffer);
            swapchain_images.push_back(new Gl3Image);
            fences.push_back(new Gl3Fence);
        }
    }

    uint32_t Gl3Swapchain::acquire_next_swapchain_image(Semaphore* signal_semaphore) {
        Gl3Semaphore* gl_semaphore = static_cast<Gl3Semaphore*>(signal_semaphore);
        std::unique_lock lck(gl_semaphore->mutex);
        gl_semaphore->signaled = true;
        gl_semaphore->cv.notify_all();

        const uint32_t ret_val = cur_frame;
        cur_frame++;
        if(cur_frame >= num_frames) {
            cur_frame = 0;
        }

        return ret_val;
    }

    void Gl3Swapchain::present(uint32_t /* image_idx */, const std::vector<Semaphore*> wait_semaphores) {
        for(Semaphore* semaphore : wait_semaphores) {
            Gl3Semaphore* gl_semaphore = static_cast<Gl3Semaphore*>(semaphore);
            std::unique_lock lck(gl_semaphore->mutex);
            gl_semaphore->cv.wait(lck, [&gl_semaphore] { return gl_semaphore->signaled; });
        }

        glFlush();
    }
} // namespace nova::renderer::rhi
