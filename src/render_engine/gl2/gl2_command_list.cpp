/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include "gl2_command_list.hpp"

#include "gl2_structs.hpp"

namespace nova::renderer::rhi {
	Gl2Command::~Gl2Command() {
		switch (type) {
		case Gl2CommandType::BufferCopy:
			buffer_copy.~Gl2BufferCopyCommand();
			break;
		case Gl2CommandType::ExecuteCommandLists:
			execute_command_lists.~Gl2ExecuteCommandListsCommand();
			break;
		case Gl2CommandType::BeginRenderpass:
			begin_renderpass.~Gl2BeginRenderpassCommand();
			break;

		case Gl2CommandType::EndRenderpass:
			// TODO
			break;

		case Gl2CommandType::BindPipeline:
			bind_pipeline.~Gl2BindPipelineCommand();
			break;

		case Gl2CommandType::BindMaterial:
			bind_material.~Gl2BindMaterialCommand();
			break;

		case Gl2CommandType::BindVertexBuffers:
			bind_vertex_buffers.~Gl2BindVertexBuffersCommand();
		    break;

		case Gl2CommandType::BindIndexBuffer:
			bind_index_buffer.~Gl2BindIndexBufferCommand();
		    break;

		case Gl2CommandType::DrawIndexedMesh:
			draw_indexed_mesh.~Gl2DrawIndexedMeshCommand();
		    break;
		}
	}

	rhi::Gl2CommandList::Gl2CommandList() {
		// TODO: maintain an average of the number of commands per command list, and allocate enough commands for like 90% of all command
		// lists
		commands.reserve(128);
	}

	void Gl2CommandList::resource_barriers([[maybe_unused]] PipelineStageFlags stages_before_barrier,
		[[maybe_unused]] PipelineStageFlags stages_after_barrier,
		[[maybe_unused]] const std::vector<ResourceBarrier>& barriers) {
		// Don't need to do anything whoop
	}

	void Gl2CommandList::copy_buffer(Buffer* destination_buffer,
		const uint64_t destination_offset,
		Buffer* source_buffer,
		const uint64_t source_offset,
		const uint64_t num_bytes) {
		GL2Buffer* dst_buf = reinterpret_cast<GL2Buffer*>(destination_buffer);
		GL2Buffer* src_buf = reinterpret_cast<GL2Buffer*>(source_buffer);

		commands.emplace_back();

		Gl2Command& copy_command = commands.front();
		copy_command.type = Gl2CommandType::BufferCopy;
		copy_command.buffer_copy.destination_buffer = dst_buf->id;
		copy_command.buffer_copy.destination_offset = destination_offset;
		copy_command.buffer_copy.source_buffer = src_buf->id;
		copy_command.buffer_copy.source_offset = source_offset;
		copy_command.buffer_copy.num_bytes = num_bytes;
	}

	void Gl2CommandList::execute_command_lists(const std::vector<CommandList*>& lists) {
		commands.emplace_back();

		Gl2Command& execute_lists_command = commands.front();
		execute_lists_command.execute_command_lists.lists_to_execute = lists;
	}

	void Gl2CommandList::begin_renderpass([[maybe_unused]] Renderpass* renderpass, Framebuffer* framebuffer) {
		GL2Framebuffer* gl_framebuffer = reinterpret_cast<GL2Framebuffer*>(framebuffer);

		commands.emplace_back();

		Gl2Command& renderpass_command = commands.front();
		renderpass_command.type = Gl2CommandType::BeginRenderpass;
		renderpass_command.begin_renderpass.framebuffer = gl_framebuffer->framebuffer;
	}

	std::vector<Gl2Command> Gl2CommandList::get_commands() const { return commands; }
} // namespace nova::renderer
