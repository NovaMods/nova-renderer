/*!
 * \author David
 * \date 05-Jul-16.
 */

#include <easylogging++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../nova_renderer.h"
#include "uniform_buffer_store.h"

namespace nova {
    uniform_buffer_store::uniform_buffer_store(std::shared_ptr<render_context> context) {
        auto per_model_buffer_create_info = vk::BufferCreateInfo()
                .setSize(10000 * sizeof(glm::mat4))
                .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setQueueFamilyIndexCount(1)
                .setPQueueFamilyIndices(&context->graphics_family_idx);

        auto uniform_buffer_offset_alignment = context->gpu.props.limits.minUniformBufferOffsetAlignment;
        per_model_resources_buffer = std::make_shared<auto_buffer>("NovaPerModelUBO", context, per_model_buffer_create_info, uniform_buffer_offset_alignment, true);



		vk::BufferCreateInfo create_per_frame_ubo = vk::BufferCreateInfo()
				.setSize(sizeof(per_frame_uniforms))
				.setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer)
				.setSharingMode(vk::SharingMode::eExclusive)
				.setQueueFamilyIndexCount(1)
				.setPQueueFamilyIndices(&context->graphics_family_idx);

		auto per_frame_ubo = uniform_buffer("NovaPerFrameUBO", context, create_per_frame_ubo, 912, true);
		add_buffer(per_frame_ubo);
    }

    void uniform_buffer_store::on_config_change(nlohmann::json &new_config) {
        // We'll probably also want to update the per frame uniforms so that we have the correct aspect ratio and
        // whatnot
    }

    void uniform_buffer_store::on_config_loaded(nlohmann::json &config) {}
	

	std::shared_ptr<auto_buffer> uniform_buffer_store::get_per_model_buffer() {
		return per_model_resources_buffer;
	}

	void uniform_buffer_store::add_buffer(uniform_buffer & new_buffer) {
        LOG(DEBUG) << "Adding buffer " << new_buffer.get_name();
        // CLion yells about this but it's fine
		buffers.insert(std::make_pair(new_buffer.get_name(), std::move(new_buffer)));
	}

	const bool uniform_buffer_store::is_buffer_known(std::string buffer_name) const {
		if(buffer_name == "NovaPerModelUBO") {
			// Special snowflake that's handled differently because stupidly flexible renderers are hard
			return true;
		}

		auto ss = std::stringstream();
		ss << "Looking for buffer " << buffer_name << " in known buffers: ";
		for(const auto& buffer : buffers) {
		    ss << buffer.first << ", ";
		}
		LOG(TRACE) << ss.str();

		return buffers.find(buffer_name) != buffers.end();
	}

	uniform_buffer & uniform_buffer_store::get_buffer(std::string buffer_name) {
		if(!is_buffer_known(buffer_name)) {
			LOG(ERROR) << "Buffer " << buffer_name << " is not known to Nova";
			throw std::runtime_error("Buffer " + buffer_name + " is not known to Nova");
		}

		if(buffer_name == "NovaPerModelUBO") {
		    // Special case ew
            return *per_model_resources_buffer.get();
		}

		return buffers.at(buffer_name);
	}
}

