#pragma once

#include <vector>

#include <RenderDevice.h>
#include <TextureUtilities.h>
#include "ShaderMacroHelper.hpp"
#include <Buffer.h>
#include <RefCntAutoPtr.hpp>
#include <BasicMath.hpp>
#include <GraphicsTypesX.hpp>

#include "graphics_shader_include.hpp"

namespace engine
{
    namespace graphics
    {
        struct SHADER_INFO
        {
            std::string name;
            std::string path;
            std::string entry_point = "main";
        };

        struct PSO_INFO
        {
            std::string name;

            Diligent::TEXTURE_FORMAT rtv_format = Diligent::TEX_FORMAT_UNKNOWN;
            Diligent::TEXTURE_FORMAT dsv_format = Diligent::TEX_FORMAT_UNKNOWN;
            Diligent::IShaderSourceInputStreamFactory* shader_source_factory = nullptr;

            SHADER_INFO vertex_shader;
            SHADER_INFO pixel_shader;

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

        std::vector<Diligent::Vertex> create_vertices(
            std::vector<Diligent::float3> positions,
            std::vector<Diligent::float3> normals,
            std::vector<Diligent::float2> textcoords
        );

        Diligent::RefCntAutoPtr<Diligent::IBuffer> create_vertex_buffer(
            Diligent::IRenderDevice* device,
            const std::vector<Diligent::Vertex> vertices,
            const Diligent::BIND_FLAGS bind_flags = Diligent::BIND_VERTEX_BUFFER,
            const Diligent::BUFFER_MODE mode = Diligent::BUFFER_MODE_UNDEFINED
        );

        Diligent::RefCntAutoPtr<Diligent::IBuffer> create_indice_buffer(
            Diligent::IRenderDevice* device,
            const std::vector<Diligent::Uint32> indices,
            const Diligent::BIND_FLAGS bind_flags = Diligent::BIND_INDEX_BUFFER,
            const Diligent::BUFFER_MODE mode = Diligent::BUFFER_MODE_UNDEFINED
        );

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> create_pipeline_state(
            Diligent::IRenderDevice* device,
            const PSO_INFO& pso_info,
            const Diligent::Uint32 num_textures,
            const Diligent::Uint32 num_samplers
        );

        Diligent::RefCntAutoPtr<Diligent::ITexture> load_texture(
            Diligent::IRenderDevice* device, 
            const std::string& texture_path
        );
    }
}
