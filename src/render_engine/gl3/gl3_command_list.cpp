/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include "gl3_command_list.hpp"

#include "gl3_structs.hpp"

namespace nova::renderer::rhi {
	Gl3Command::~Gl3Command() {
		switch (type) {
		case Gl3CommandType::BufferCopy:
			buffer_copy.~Gl3BufferCopyCommand();
			break;
		case Gl3CommandType::ExecuteCommandLists:
			execute_command_lists.~Gl3ExecuteCommandListsCommand();
			break;
		case Gl3CommandType::BeginRenderpass:
			begin_renderpass.~Gl3BeginRenderpassCommand();
			break;

		case Gl3CommandType::EndRenderpass:
			// TODO
			break;

		case Gl3CommandType::BindPipeline:
			bind_pipeline.~Gl3BindPipelineCommand();
			break;

		case Gl3CommandType::BindMaterial:
			bind_material.~Gl3BindMaterialCommand();
			break;

		case Gl3CommandType::BindVertexBuffers:
			bind_vertex_buffers.~Gl3BindVertexBuffersCommand();
		    break;

		case Gl3CommandType::BindIndexBuffer:
			bind_index_buffer.~Gl3BindIndexBufferCommand();
		    break;

		case Gl3CommandType::DrawIndexedMesh:
			draw_indexed_mesh.~Gl3DrawIndexedMeshCommand();
		    break;
		}
	}

	rhi::Gl3CommandList::Gl3CommandList() {
		// TODO: maintain an average of the number of commands per command list, and allocate enough commands for like 90% of all command
		// lists
		commands.reserve(128);
	}

	void Gl3CommandList::resource_barriers([[maybe_unused]] PipelineStageFlags stages_before_barrier,
		[[maybe_unused]] PipelineStageFlags stages_after_barrier,
		[[maybe_unused]] const std::vector<ResourceBarrier>& barriers) {
		// Don't need to do anything whoop
	}

	void Gl3CommandList::copy_buffer(Buffer* destination_buffer,
		const uint64_t destination_offset,
		Buffer* source_buffer,
		const uint64_t source_offset,
		const uint64_t num_bytes) {
		Gl3Buffer* dst_buf = reinterpret_cast<Gl3Buffer*>(destination_buffer);
		Gl3Buffer* src_buf = reinterpret_cast<Gl3Buffer*>(source_buffer);

		commands.emplace_back();

		Gl3Command& copy_command = commands.front();
		copy_command.type = Gl3CommandType::BufferCopy;
		copy_command.buffer_copy.destination_buffer = dst_buf->id;
		copy_command.buffer_copy.destination_offset = destination_offset;
		copy_command.buffer_copy.source_buffer = src_buf->id;
		copy_command.buffer_copy.source_offset = source_offset;
		copy_command.buffer_copy.num_bytes = num_bytes;
	}

	void Gl3CommandList::execute_command_lists(const std::vector<CommandList*>& lists) {
		commands.emplace_back();

		Gl3Command& execute_lists_command = commands.front();
		execute_lists_command.execute_command_lists.lists_to_execute = lists;
	}

	void Gl3CommandList::begin_renderpass([[maybe_unused]] Renderpass* renderpass, Framebuffer* framebuffer) {
		Gl3Framebuffer* gl_framebuffer = reinterpret_cast<Gl3Framebuffer*>(framebuffer);

		commands.emplace_back();

		Gl3Command& renderpass_command = commands.front();
		renderpass_command.type = Gl3CommandType::BeginRenderpass;
		renderpass_command.begin_renderpass.framebuffer = gl_framebuffer->id;
	}

	std::vector<Gl3Command> Gl3CommandList::get_commands() const { return commands; }
} // namespace nova::renderer
