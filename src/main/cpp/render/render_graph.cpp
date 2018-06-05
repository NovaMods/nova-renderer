/*!
 * \author ddubois 
 * \date 25-Feb-18.
 */

#include "render_graph.h"
#include <stdexcept>
#include <unordered_set>
#include <easylogging++.h>

namespace nova {
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
    void add_dependent_passes(const std::string &pass_name,
                              const std::unordered_map<std::string, render_pass>& passes,
                              std::vector<std::string>& ordered_passes,
                              const std::unordered_map<std::string, std::vector<std::string>>& resource_to_write_pass,
                              uint32_t depth);

    std::vector<std::string> order_passes(const std::unordered_map<std::string, render_pass> &passes) {
        LOG(INFO) << "Ordering passes";
        auto ordered_passes = std::vector<std::string>{};

        /*
         * Build some accelleration structures
         */

        LOG(INFO) << "Collecting passes that write to each resource...";
        // Maps from resource name to pass that writes to that resource, then from resource name to pass that reads from
        // that resource
        auto resource_to_write_pass = std::unordered_map<std::string, std::vector<std::string>>{};

        for(const auto& item : passes) {
            const render_pass& pass = item.second;

            for(const auto& output : pass.texture_outputs.value()) {
                resource_to_write_pass[output.name].push_back(pass.name);
            }
        }

        /*
         * Initial ordering of passes
         */

        LOG(INFO) << "First pass at ordering passes...";
        // The passes, in simple dependency order
        if(resource_to_write_pass.find("Backbuffer") == resource_to_write_pass.end()) {
            LOG(ERROR) << "This render graph does not write to the backbuffer. Unable to load this shaderpack because it can't render anything";
            throw std::runtime_error("no backbuffer");

        } else {    // While the throw should make it clear that this is a separate branch, I forgot so here's an else
            auto backbuffer_writes = resource_to_write_pass["Backbuffer"];
            ordered_passes.insert(ordered_passes.end(), backbuffer_writes.begin(), backbuffer_writes.end());

            for(const auto& pass : backbuffer_writes) {
                add_dependent_passes(pass, passes, ordered_passes, resource_to_write_pass, 1);
            }

            std::reverse(ordered_passes.begin(), ordered_passes.end());

            // We're going to loop through the original list of passes and remove them from the original list of passes
            // We want to keep the original passes around

            // This code taken from `RenderGraph::filter_passes` in the Granite engine
            // It loops through the ordered passes. When it sees the name of a new pass, it writes the pass to
            // ordered_passes and increments the write position. After all the passes are written, we remove all the
            // passes after the last one we wrote to, shrinking the list of ordered passes to only include the exact passes we want
            std::unordered_set<std::string> seen;

            auto output_itr = ordered_passes.begin();
            for (const auto& pass : ordered_passes)
            {
                if (!seen.count(pass))
                {
                    *output_itr = pass;
                    seen.insert(pass);
                    ++output_itr;
                }
            }
            ordered_passes.erase(output_itr, ordered_passes.end());
        }

        // Granite does some reordering to try and find a submission order that has the fewest pipeline barriers. While
        // doing that may be useful in the future, the fact that I'm using OpenGL means I'm not going to worry about
        // that for noe

        return ordered_passes;
    }

    void add_dependent_passes(const std::string &pass_name,
                              const std::unordered_map<std::string, render_pass>& passes,
                              std::vector<std::string>& ordered_passes,
                              const std::unordered_map<std::string, std::vector<std::string>>& resource_to_write_pass,
                              const uint32_t depth) {
        if(depth > passes.size()) {
            LOG(ERROR) << "Circular render graph detected! Please fix your render graph to not have circular dependencies";
            throw std::runtime_error("circular graph");
        }

        const auto& pass = passes.at(pass_name);

        // Add all the passes that this pass is dependent on
        if(pass.dependencies) {
            for(const auto& dependency : pass.dependencies.value()) {
                ordered_passes.push_back(dependency);
                add_dependent_passes(dependency, passes, ordered_passes, resource_to_write_pass, depth + 1);
            }
        }

        if(pass.texture_inputs) {
            const input_textures& all_inputs = pass.texture_inputs.value();
            for(const auto& texture_name : all_inputs.bound_textures) {
                if(resource_to_write_pass.find(texture_name) == resource_to_write_pass.end()) {
                    // TODO: Ignore the implicitly defined resources
                    LOG(ERROR) << "Pass " << pass_name << " reads from resource " << texture_name << ", but nothing writes to it";

                } else {
                    const auto &write_passes = resource_to_write_pass.at(texture_name);
                    ordered_passes.insert(ordered_passes.end(), write_passes.begin(), write_passes.end());

                    for(const auto& write_pass : write_passes) {
                        add_dependent_passes(write_pass, passes, ordered_passes, resource_to_write_pass, depth + 1);
                    }
                }
            }

            for(const auto& texture_name : all_inputs.color_attachments) {
                if(resource_to_write_pass.find(texture_name) == resource_to_write_pass.end()) {
                    // TODO: Ignore the implicitly defined resources
                    LOG(ERROR) << "Pass " << pass_name << " reads from resource " << texture_name << ", but nothing writes to it";

                } else {
                    const auto &write_passes = resource_to_write_pass.at(texture_name);
                    ordered_passes.insert(ordered_passes.end(), write_passes.begin(), write_passes.end());

                    for(const auto& write_pass : write_passes) {
                        add_dependent_passes(write_pass, passes, ordered_passes, resource_to_write_pass, depth + 1);
                    }
                }
            }
        }
    }
}
