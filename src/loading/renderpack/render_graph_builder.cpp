#include "render_graph_builder.hpp"

#include <Tracy.hpp>
#include <rx/core/algorithm/max.h>
#include <rx/core/algorithm/min.h>
#include <rx/core/log.h>

#include "nova_renderer/constants.hpp"

namespace nova::renderer::renderpack {
    RX_LOG("RenderGraphBuilder", logger);

    /*!
     * \brief Adds all the passes that `pass_name` depends on to the list of ordered passes
     *
     * This method performs a depth-first traversal of the pass tree. It shouldn't matter whether we do depth or
     * breadth first, but depth first feels cleaner
     *
     * \param pass_name The passes that were just added to the list of ordered passes
     * \param passes A map from pass name to pass. Useful for the explicit dependencies of a pass
     * \param ordered_passes The passes in submissions order... almost. When this function adds to ordered_passes the
     * list has a lot of duplicates. They're removed in a later step
     * \param resource_to_write_pass A map from resource name to list of passes that write to that resource. Useful for
     * resolving the implicit dependencies of a pass
     * \param depth The depth in the tree that we're at. If this number ever grows bigger than the total number of
     * passes, there's a circular dependency somewhere in the render graph. This is Bad and we hate it
     */
    void add_dependent_passes(const std::string& pass_name,
                              const std::unordered_map<std::string, RenderPassCreateInfo>& passes,
                              std::vector<std::string>& ordered_passes,
                              const std::unordered_map<std::string, std::vector<std::string>>& resource_to_write_pass,
                              uint32_t depth);

    bool Range::has_writer() const { return first_write_pass <= last_write_pass; }

    bool Range::has_reader() const { return first_read_pass <= last_read_pass; }

    bool Range::is_used() const { return has_writer() || has_reader(); }

    bool Range::can_alias() const {
        // If we read before we have completely written to a resource we need to preserve it, so no alias is possible.
        return !(has_reader() && has_writer() && first_read_pass <= first_write_pass);
    }

    unsigned Range::last_used_pass() const {
        unsigned last_pass = 0;
        if(has_writer()) {
            last_pass = rx::algorithm::max(last_pass, last_write_pass);
        }
        if(has_reader()) {
            last_pass = rx::algorithm::max(last_pass, last_read_pass);
        }
        return last_pass;
    }

    unsigned Range::first_used_pass() const {
        unsigned first_pass = ~0U;
        if(has_writer()) {
            first_pass = rx::algorithm::min(first_pass, first_write_pass);
        }
        if(has_reader()) {
            first_pass = rx::algorithm::min(first_pass, first_read_pass);
        }
        return first_pass;
    }

    bool Range::is_disjoint_with(const Range& other) const {
        if(!is_used() || !other.is_used()) {
            return false;
        }
        if(!can_alias() || !other.can_alias()) {
            return false;
        }

        const bool left = last_used_pass() < other.first_used_pass();
        const bool right = other.last_used_pass() < first_used_pass();
        return left || right;
    }

    ntl::Result<std::vector<RenderPassCreateInfo>> order_passes(const std::vector<RenderPassCreateInfo>& passes) {
        MTR_SCOPE("Renderpass", "order_passes");

        logger->verbose("Executing Pass Scheduler");

        std::unordered_map<std::string, RenderPassCreateInfo> render_passes_to_order;
        passes.each_fwd([&](const RenderPassCreateInfo& create_info) { render_passes_to_order.insert(create_info.name, create_info); });

        std::vector<std::string> ordered_passes;
        ordered_passes.reserve(passes.size());

        /*
         * Build some acceleration structures
         */

        logger->verbose("Collecting passes that write to each resource...");
        // Maps from resource name to pass that writes to that resource, then from resource name to pass that reads from
        // that resource
        auto resource_to_write_pass = std::unordered_map<std::string, std::vector<std::string>>{};

        passes.each_fwd([&](const RenderPassCreateInfo& pass) {
            pass.texture_outputs.each_fwd([&](const TextureAttachmentInfo& output) {
                auto* write_pass_list = resource_to_write_pass.find(output.name);
                if(!write_pass_list) {
                    write_pass_list = resource_to_write_pass.insert(output.name, {});
                }
                write_pass_list->push_back(pass.name);
            });

            pass.output_buffers.each_fwd([&](const std::string& buffer_name) {
                auto* write_pass_list = resource_to_write_pass.find(buffer_name);
                if(!write_pass_list) {
                    write_pass_list = resource_to_write_pass.insert(buffer_name, {});
                }
                write_pass_list->push_back(pass.name);
            });
        });

        /*
         * Initial ordering of passes
         */

        logger->verbose("First pass at ordering passes...");
        // The passes, in simple dependency order
        if(resource_to_write_pass.find(BACKBUFFER_NAME) == nullptr) {
            logger->error(
                "This render graph does not write to the backbuffer. Unable to load this renderpack because it can't render anything");
            return ntl::Result<std::vector<RenderPassCreateInfo>>(ntl::NovaError("Failed to order passes because no backbuffer was found"));
        }

        const auto& backbuffer_writes = *resource_to_write_pass.find(BACKBUFFER_NAME);
        ordered_passes += backbuffer_writes;

        backbuffer_writes.each_fwd([&](const std::string& pass_name) {
            add_dependent_passes(pass_name, render_passes_to_order, ordered_passes, resource_to_write_pass, 1);
        });

        // We're going to loop through the original list of passes and remove them from the original list of passes
        // We want to keep the original passes around

        // This code taken from `RenderGraph::filter_passes` in the Granite engine
        // It loops through the ordered passes. When it sees the name of a new pass, it writes the pass to
        // ordered_passes and increments the write position. After all the passes are written, we remove all the
        // passes after the last one we wrote to, shrinking the list of ordered passes to only include the exact passes we want

        std::vector<std::string> unique_passes;

        ordered_passes.each_rev([&](const std::string& pass_name) {
            if(unique_passes.find(pass_name) == std::vector<std::string>::k_npos) {
                unique_passes.push_back(pass_name);
            }
        });

        ordered_passes = unique_passes;

        // Granite does some reordering to try and find a submission order that has the fewest pipeline barriers. Not
        // gonna worry about that now

        std::vector<RenderPassCreateInfo> passes_in_submission_order;
        passes_in_submission_order.reserve(ordered_passes.size());

        ordered_passes.each_fwd(
            [&](const std::string& pass_name) { passes_in_submission_order.push_back(*render_passes_to_order.find(pass_name)); });

        return ntl::Result(passes_in_submission_order);
    }

    void add_dependent_passes(const std::string& pass_name,
                              const std::unordered_map<std::string, RenderPassCreateInfo>& passes,
                              std::vector<std::string>& ordered_passes,
                              const std::unordered_map<std::string, std::vector<std::string>>& resource_to_write_pass,
                              const uint32_t depth) {
        if(depth > passes.size()) {
            logger->error("Circular render graph detected! Please fix your render graph to not have circular dependencies");
        }

        const auto& pass = *passes.find(pass_name);

        pass.texture_inputs.each_fwd([&](const std::string& texture_name) {
            if(const auto write_passes = resource_to_write_pass.find(texture_name); write_passes == nullptr) {
                // TODO: Ignore the implicitly defined resources
                logger->error("Pass %s reads from resource %s, but nothing writes to it", pass_name, texture_name);
            } else {
                ordered_passes += *write_passes;

                write_passes->each_fwd([&](const std::string& write_pass) {
                    add_dependent_passes(write_pass, passes, ordered_passes, resource_to_write_pass, depth + 1);
                });
            }
        });

        pass.input_buffers.each_fwd([&](const std::string& buffer_name) {
            if(const auto& write_passes = resource_to_write_pass.find(buffer_name); write_passes == nullptr) {
                logger->error("Pass %s reads from buffer %s, but no passes write to it", pass_name, buffer_name);
            } else {
                ordered_passes += *write_passes;

                write_passes->each_fwd([&](const std::string& write_pass) {
                    add_dependent_passes(write_pass, passes, ordered_passes, resource_to_write_pass, depth + 1);
                });
            }
        });
    }

    void determine_usage_order_of_textures(const std::vector<RenderPassCreateInfo>& passes,
                                           std::unordered_map<std::string, Range>& resource_used_range,
                                           std::vector<std::string>& resources_in_order) {
        uint32_t pass_idx = 0;
        passes.each_fwd([&](const RenderPassCreateInfo& pass) {
            // color attachments
            pass.texture_inputs.each_fwd([&](const std::string& input) {
                const auto tex_range = resource_used_range.find(input);

                if(pass_idx < tex_range->first_write_pass) {
                    tex_range->first_write_pass = pass_idx;
                } else if(pass_idx > tex_range->last_write_pass) {
                    tex_range->last_write_pass = pass_idx;
                }

                if(resources_in_order.find(input) == std::vector<std::string>::k_npos) {
                    resources_in_order.push_back(input);
                }
            });

            pass.texture_outputs.each_fwd([&](const TextureAttachmentInfo& output) {
                const auto tex_range = resource_used_range.find(output.name);

                if(pass_idx < tex_range->first_write_pass) {
                    tex_range->first_write_pass = pass_idx;
                } else if(pass_idx > tex_range->last_write_pass) {
                    tex_range->last_write_pass = pass_idx;
                }

                if(resources_in_order.find(output.name) == std::vector<std::string>::k_npos) {
                    resources_in_order.push_back(output.name);
                }
            });

            pass_idx++;
        });
    }

    std::unordered_map<std::string, std::string> determine_aliasing_of_textures(const std::unordered_map<std::string, TextureCreateInfo>& textures,
                                                                   const std::unordered_map<std::string, Range>& resource_used_range,
                                                                   const std::vector<std::string>& resources_in_order) {
        std::unordered_map<std::string, std::string> aliases;

        for(size_t i = 0; i < resources_in_order.size(); i++) {
            const auto& to_alias_name = resources_in_order[i];
            logger->verbose("Determining if we can alias `%s`. Does it exist? %d",
                            to_alias_name,
                            (textures.find(to_alias_name) != nullptr));

            if(to_alias_name == BACKBUFFER_NAME || to_alias_name == SCENE_OUTPUT_RT_NAME) {
                // Yay special cases!
                continue;
            }

            const auto& to_alias_format = textures.find(to_alias_name)->format;

            // Only try to alias with lower-indexed resources
            for(size_t j = 0; j < i; j++) {
                logger->verbose("Trying to alias it with resource at index %zu out of %zu", j, resources_in_order.size());
                const std::string& try_alias_name = resources_in_order[j];
                if(resource_used_range.find(to_alias_name)->is_disjoint_with(*resource_used_range.find(try_alias_name))) {
                    // They can be aliased if they have the same format
                    const auto& try_alias_format = textures.find(try_alias_name)->format;
                    if(to_alias_format == try_alias_format) {
                        aliases.insert(to_alias_name, try_alias_name);
                    }
                }
            }
        }

        return aliases;
    }

} // namespace nova::renderer::renderpack
