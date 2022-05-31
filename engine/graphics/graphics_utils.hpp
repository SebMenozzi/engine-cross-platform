#pragma once

#include <vector>

#include <RenderDevice.h>
#include <TextureUtilities.h>
#include <Buffer.h>
#include <RefCntAutoPtr.hpp>
#include <BasicMath.hpp>
#include <GraphicsTypesX.hpp>

namespace engine
{
    namespace graphics
    {
        struct VERTEX_DATA
        {
            std::vector<Diligent::float3> positions = {};
            std::vector<Diligent::float3> normals = {};
            std::vector<Diligent::float2> texcoords = {};
        };

        struct OBJECT_DATA
        {
            VERTEX_DATA vertex_data = {};
            std::vector<Diligent::Uint32> indices = {};
        };

        enum VERTEX_COMPONENT_FLAGS : Diligent::Uint32
        {
            VERTEX_COMPONENT_FLAG_NONE = 0x00,
            VERTEX_COMPONENT_FLAG_POSITION = 0x01,
            VERTEX_COMPONENT_FLAG_NORMAL = 0x02,
            VERTEX_COMPONENT_FLAG_TEXCOORD = 0x04,
            VERTEX_COMPONENT_FLAG_POSITION_TEXCOORD = VERTEX_COMPONENT_FLAG_POSITION | VERTEX_COMPONENT_FLAG_TEXCOORD,
            VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD = VERTEX_COMPONENT_FLAG_POSITION | VERTEX_COMPONENT_FLAG_NORMAL | VERTEX_COMPONENT_FLAG_TEXCOORD
        };

        struct SHADER_INFO
        {
            std::string name;
            std::string path;
            std::string entry_point = "main";
            bool use_combined_texture_samplers = true;
        };

        struct PSO_INFO
        {
            std::string name;

            Diligent::TEXTURE_FORMAT rtv_format = Diligent::TEX_FORMAT_UNKNOWN;
            Diligent::TEXTURE_FORMAT dsv_format = Diligent::TEX_FORMAT_UNKNOWN;
            Diligent::IShaderSourceInputStreamFactory* shader_source_factory = nullptr;

            SHADER_INFO vertex_shader;
            SHADER_INFO pixel_shader;

            VERTEX_COMPONENT_FLAGS components = VERTEX_COMPONENT_FLAG_NONE;
            Diligent::LayoutElement* layout_elements = nullptr;
            Diligent::Uint32 nb_layout_elements = 0;
            Diligent::Uint8 sample_count = 1;
            Diligent::PRIMITIVE_TOPOLOGY topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            Diligent::CULL_MODE cull_mode = Diligent::CULL_MODE_NONE;
            Diligent::FILL_MODE fill_mode = Diligent::FILL_MODE_SOLID;
            Diligent::Bool front_counter_clockwise = false;
            bool depth_enable = false;
            bool depth_write_enable = false;

            const Diligent::ShaderResourceVariableDesc* variables = nullptr;
            Diligent::Uint32 nb_variables = 0;
            
            const Diligent::ImmutableSamplerDesc* immutable_samplers = nullptr;
            Diligent::Uint32 nb_immutable_samplers = 0;
        };

        Diligent::RefCntAutoPtr<Diligent::IBuffer> create_vertex_buffer(
            Diligent::IRenderDevice* device,
            VERTEX_DATA vertex_data,
            VERTEX_COMPONENT_FLAGS components,
            Diligent::BIND_FLAGS bind_flags = Diligent::BIND_VERTEX_BUFFER,
            Diligent::BUFFER_MODE mode = Diligent::BUFFER_MODE_UNDEFINED
        );

        Diligent::RefCntAutoPtr<Diligent::IBuffer> create_index_buffer(
            Diligent::IRenderDevice* device,
            const std::vector<Diligent::Uint32> indices,
            Diligent::BIND_FLAGS bind_flags = Diligent::BIND_INDEX_BUFFER,
            Diligent::BUFFER_MODE mode = Diligent::BUFFER_MODE_UNDEFINED
        );

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> create_pipeline_state(
            Diligent::IRenderDevice* device,
            const PSO_INFO& pso_info
        );

        Diligent::RefCntAutoPtr<Diligent::ITexture> load_texture(
            Diligent::IRenderDevice* device, 
            const std::string& texture_path
        );
    }
}
