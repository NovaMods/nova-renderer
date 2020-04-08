#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "nova_renderer/rhi/resource_binder.hpp"

#include "rx/core/map.h"

// Fix WinAPI cause Rex broke it
#define interface struct

#include "d3d12_enums.hpp"
#include "d3d12_structs.hpp"

namespace nova::renderer::rhi {
    struct D3D12Pipeline;

    struct D3D12RootDescriptor {
        D3D12ResourceType type;
        rx::string binding_name;
    };

    struct D3D12DescriptorTable {
        D3D12_GPU_DESCRIPTOR_HANDLE descriptor_table_start;
    };

    struct D3D12RootParameter {
        enum class Type {
            RootConstant,
            RootDescriptor,
            DescriptorTable,
        };

        Type type;

        union {
            D3D12RootDescriptor root_descriptor;
            D3D12DescriptorTable descriptor_table;
        };
    };

    struct D3D12Descriptor {
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
        D3D12ResourceType type;

        /*!
         * \brief Size of the array element of a structured buffer
         *
         * This will often be 0 because not every binding will be a structured buffer. However, for bindings which _are_ a structured
         * buffer, this member is the size, in bytes, of the structure
         */
        UINT array_element_size = 0;
    };

    /*!
     * \brief Allows the user to bind resources to a D3D12 command list in a sane way
     *
     * This class makes a lot of assumptions that all the input data is correct
     *
     * Important note: descriptors for arrays in the shader MUST have enough free space in the descriptor heap to hold the full array. For
     * things like Nova's textures array, this probably means that you need space for 128k descriptors? I don't have a solid idea of how
     * many textures Nova will support at once, but it'll probably be a few hundred thousand at least
     */
    class D3D12ResourceBinder : public RhiResourceBinder {
    public:
        /*!
         * \brief Creates a new D3D12ResourceBinder
         *
         * \param allocator The allocator that this instance should use internally
         * \param descriptor_size_in The size of one descriptor
         * \param device_in The ID3D12Device which will consume these resource bindings
         * \param root_signature_in The root signature that this resource binder wil bind resources to
         * \param root_parameters_in A description of the root parameters that this resource binder will need to update. Includes
         * information about _what_ to bind to each root signature slot - whether to directly bind a named binding (and which binding to
         * bind), to bind a descriptor table (and the offset into the descriptor heap where that table can be found), or to bind a root
         * constant - although at the time of writing D3D12ResourceBinder has no support for root constants
         * \param descriptor_table_bindings_in All the descriptors in descriptor tables managed by this resource binder
         */
        D3D12ResourceBinder(rx::memory::allocator& allocator,
                            INT descriptor_size_in,
                            Microsoft::WRL::ComPtr<ID3D12Device> device_in,
                            Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_in,
                            rx::vector<D3D12RootParameter> root_parameters_in,
                            rx::map<rx::string, D3D12Descriptor> descriptor_table_bindings_in);

        ~D3D12ResourceBinder() override = default;

#pragma region RhiResourceBinder
        void bind_image(const rx::string& binding_name, RhiImage* image) override;

        void bind_buffer(const rx::string& binding_name, RhiBuffer* buffer) override;

        void bind_sampler(const rx::string& binding_name, RhiSampler* sampler) override;

        void bind_image_array(const rx::string& binding_name, const rx::vector<RhiImage*>& images) override;

        void bind_buffer_array(const rx::string& binding_name, const rx::vector<RhiBuffer*>& buffers) override;

        void bind_sampler_array(const rx::string& binding_name, const rx::vector<RhiSampler*>& samplers) override;
#pragma endregion

        void update_descriptors();

        void bind_descriptors_to_command_list(ID3D12GraphicsCommandList* cmds) const;

        void bind_root_descriptor(uint32_t slot_idx, const D3D12RootDescriptor& root_descriptor, ID3D12GraphicsCommandList* cmds) const;

    private:
        rx::memory::allocator* internal_allocator;

        rx::map<rx::string, rx::vector<D3D12Image*>> bound_images;
        rx::map<rx::string, rx::vector<D3D12Buffer*>> bound_buffers;
        rx::map<rx::string, rx::vector<D3D12Sampler*>> bound_samplers;

        Microsoft::WRL::ComPtr<ID3D12Device> device;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;

        rx::vector<D3D12RootParameter> root_parameters;

        /*!
         * \brief Map of all the bindings which are part of a descriptor table
         */
        rx::map<rx::string, D3D12Descriptor> descriptor_table_bindings;

        INT descriptor_size;

        bool is_dirty = false;

        void create_srv_array(const rx::vector<D3D12Image*>& images, const D3D12Descriptor& start_descriptor) const;

        void create_uav_array(const rx::vector<D3D12Image*>& images, const D3D12Descriptor& start_descriptor) const;

        void create_cbv_array(const rx::vector<D3D12Buffer*>& buffers, const D3D12Descriptor& start_descriptor) const;

        void create_srv_array(const rx::vector<D3D12Buffer*>& buffers, const D3D12Descriptor& start_descriptor) const;

        void create_uav_array(const rx::vector<D3D12Buffer*>& buffers, const D3D12Descriptor& start_descriptor) const;
    };
} // namespace nova::renderer::rhi
