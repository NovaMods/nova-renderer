/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include "gl2_command_list.hpp"

#include "gl2_structs.hpp"

namespace nova::renderer::rhi {
	gl2_command::~gl2_command() {
		switch (type) {
		case gl2_command_type::BUFFER_COPY:
			buffer_copy.~gl2_buffer_copy_command();
			break;
		case gl2_command_type::EXECUTE_COMMAND_LISTS:
			execute_command_lists.~gl2_execute_command_lists_command();
			break;
		case gl2_command_type::BEGIN_RENDERPASS:
			begin_renderpass.~gl2_begin_renderpass_command();
			break;

		case gl2_command_type::END_RENDERPASS:
			// TODO
			break;

		case gl2_command_type::BIND_PIPELINE:
			bind_pipeline.~gl2_bind_pipeline_command();
			break;

		case gl2_command_type::BIND_MATERIAL:
			bind_material.~gl2_bind_material_command();
			break;

		case gl2_command_type::BIND_VERTEX_BUFFERS:
			bind_vertex_buffers.~gl2_bind_vertex_buffers_command();
		    break;

		case gl2_command_type::BIND_INDEX_BUFFER:
			bind_index_buffer.~gl2_bind_index_buffer_command();
		    break;

		case gl2_command_type::DRAW_INDEXED_MESH:
			draw_indexed_mesh.~gl2_draw_indexed_mesh_command();
		    break;
		}
	}

	rhi::gl2_command_list::gl2_command_list() {
		// TODO: maintain an average of the number of commands per command list, and allocate enough commands for like 90% of all command
		// lists
		commands.reserve(128);
	}

	void gl2_command_list::resource_barriers([[maybe_unused]] PipelineStageFlags stages_before_barrier,
		[[maybe_unused]] PipelineStageFlags stages_after_barrier,
		[[maybe_unused]] const std::vector<ResourceBarrier>& barriers) {
		// Don't need to do anything whoop
	}

	void gl2_command_list::copy_buffer(Buffer* destination_buffer,
		const uint64_t destination_offset,
		Buffer* source_buffer,
		const uint64_t source_offset,
		const uint64_t num_bytes) {
		GL2Buffer* dst_buf = reinterpret_cast<GL2Buffer*>(destination_buffer);
		GL2Buffer* src_buf = reinterpret_cast<GL2Buffer*>(source_buffer);

		commands.emplace_back();

		gl2_command& copy_command = commands.front();
		copy_command.type = gl2_command_type::BUFFER_COPY;
		copy_command.buffer_copy.destination_buffer = dst_buf->id;
		copy_command.buffer_copy.destination_offset = destination_offset;
		copy_command.buffer_copy.source_buffer = src_buf->id;
		copy_command.buffer_copy.source_offset = source_offset;
		copy_command.buffer_copy.num_bytes = num_bytes;
	}

	void gl2_command_list::execute_command_lists(const std::vector<CommandList*>& lists) {
		commands.emplace_back();

		gl2_command& execute_lists_command = commands.front();
		execute_lists_command.execute_command_lists.lists_to_execute = lists;
	}

	void gl2_command_list::begin_renderpass([[maybe_unused]] Renderpass* renderpass, Framebuffer* framebuffer) {
		GL2Framebuffer* gl_framebuffer = reinterpret_cast<GL2Framebuffer*>(framebuffer);

		commands.emplace_back();

		gl2_command& renderpass_command = commands.front();
		renderpass_command.type = gl2_command_type::BEGIN_RENDERPASS;
		renderpass_command.begin_renderpass.framebuffer = gl_framebuffer->framebuffer;
	}

	std::vector<gl2_command> gl2_command_list::get_commands() const { return commands; }
} // namespace nova::renderer
