#pragma once
#include <vector>

namespace nova::renderer::rhi {
    struct Semaphore;

    class Swapchain {
    public:
        virtual void acquire_next_swapchain_image(const std::vector<Semaphore*>& signal_semaphores) = 0;

        virtual void present_current_image(const Semaphore* wait_semaphore) = 0;
    };
}
