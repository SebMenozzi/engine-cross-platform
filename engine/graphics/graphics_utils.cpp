
#include "graphics_utils.hpp"

namespace engine
{
    namespace graphics
    {
        std::vector<Diligent::Vertex> create_vertices(
            std::vector<Diligent::float3> positions,
            std::vector<Diligent::float3> normals,
            std::vector<Diligent::float2> textcoords
        ) {
            const Diligent::Uint32 nb_vertices = Diligent::Uint32(positions.size());

            std::vector<Diligent::Vertex> vertices(nb_vertices);

            for (Diligent::Uint32 i = 0; i < nb_vertices; ++i)
            {
                vertices[i].position = positions[i];
                vertices[i].normal = normals[i];
                vertices[i].textcoord = textcoords[i];
            }

            return vertices;
        }

        Diligent::RefCntAutoPtr<Diligent::IBuffer> create_vertex_buffer(
            Diligent::IRenderDevice* device,
            std::vector<Diligent::Vertex> vertices,
            Diligent::BIND_FLAGS bind_flags,
            Diligent::BUFFER_MODE mode
        )
        {
            // Create a vertex buffer that stores vertices
            Diligent::BufferDesc vertex_buffer_desc;
            vertex_buffer_desc.Name = "Vertex buffer";
            vertex_buffer_desc.Usage = Diligent::USAGE_IMMUTABLE;
            vertex_buffer_desc.BindFlags = bind_flags;
            vertex_buffer_desc.Size = static_cast<Diligent::Uint64>(vertices.size() * sizeof(Diligent::Vertex));
            vertex_buffer_desc.Mode = mode;

            if (mode != Diligent::BUFFER_MODE_UNDEFINED)
                vertex_buffer_desc.ElementByteStride = sizeof(Diligent::Vertex);

            Diligent::BufferData vertex_buffer_data;
            vertex_buffer_data.pData = vertices.data();
            vertex_buffer_data.DataSize = vertex_buffer_desc.Size;

            Diligent::RefCntAutoPtr<Diligent::IBuffer> vertex_buffer;
            device->CreateBuffer(vertex_buffer_desc, &vertex_buffer_data, &vertex_buffer);

            return vertex_buffer;
        }

        Diligent::RefCntAutoPtr<Diligent::IBuffer> create_indice_buffer(
            Diligent::IRenderDevice* device, 
            const std::vector<Diligent::Uint32> indices,
            Diligent::BIND_FLAGS bind_flags, 
            Diligent::BUFFER_MODE mode
        )
        {
            const Diligent::Uint32 nb_indices = Diligent::Uint32(indices.size());

            Diligent::BufferDesc indice_buffer_desc;
            indice_buffer_desc.Name = "Indice buffer";
            indice_buffer_desc.Usage = Diligent::USAGE_IMMUTABLE;
            indice_buffer_desc.BindFlags = bind_flags;
            indice_buffer_desc.Size = nb_indices * sizeof(Diligent::Uint32);
            indice_buffer_desc.Mode = mode;

            if (mode != Diligent::BUFFER_MODE_UNDEFINED)
                indice_buffer_desc.ElementByteStride = sizeof(Diligent::Uint32);

            Diligent::BufferData indice_data;
            indice_data.pData = indices.data();
            indice_data.DataSize = nb_indices * sizeof(Diligent::Uint32);

            Diligent::RefCntAutoPtr<Diligent::IBuffer> indice_buffer;
            device->CreateBuffer(indice_buffer_desc, &indice_data, &indice_buffer);

            return indice_buffer;
        }

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> create_pipeline_state(
            Diligent::IRenderDevice* device,
            const PSO_INFO& pso_info,
            const Diligent::Uint32 num_textures,
            const Diligent::Uint32 num_samplers
        )
        {
            assert(device);
            
            // Pipeline state object encompasses configuration of all GPU stages
            Diligent::GraphicsPipelineStateCreateInfo pipeline_pso_info;

            // This is a graphics pipeline
            pipeline_pso_info.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
            // Pipeline state name is used by the engine to report issues.
            pipeline_pso_info.PSODesc.Name = pso_info.name.c_str();

            // Set the number of render targets to one
            pipeline_pso_info.GraphicsPipeline.NumRenderTargets = 1;
            // Set render target format which is the format of the swap chain's color buffer
            pipeline_pso_info.GraphicsPipeline.RTVFormats[0] = pso_info.rtv_format;
            // Set depth buffer format which is the format of the swap chain's back buffer
            pipeline_pso_info.GraphicsPipeline.DSVFormat = pso_info.dsv_format;
            // Set the desired number of samples
            pipeline_pso_info.GraphicsPipeline.SmplDesc.Count = pso_info.sample_count;
            // Primitive topology defines what kind of primitives will be rendered by this pipeline state
            pipeline_pso_info.GraphicsPipeline.PrimitiveTopology = pso_info.topology;
            // Indicates triangles facing the specified direction are not drawn
            pipeline_pso_info.GraphicsPipeline.RasterizerDesc.CullMode = pso_info.cull_mode;
            // Determines the fill mode to use when rendering
            pipeline_pso_info.GraphicsPipeline.RasterizerDesc.FillMode = pso_info.fill_mode;
            // Determines if a triangle is front- or back-facing. 
            // - If this parameter is TRUE, a triangle will be considered front-facing, 
            // if its vertices are counter-clockwise on the render target and considered back-facing if they are clockwise. 
            // - If this parameter is FALSE, the opposite is true.
            pipeline_pso_info.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = pso_info.front_counter_clockwise;
            // Enable depth testing
            pipeline_pso_info.GraphicsPipeline.DepthStencilDesc.DepthEnable = pso_info.depth_enable;
            // A Boolean value that indicates whether depth values can be written to the depth attachment.
            pipeline_pso_info.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = pso_info.depth_write_enable;

            Diligent::ShaderCreateInfo shader_create_info;
            // Tell the system that the shader source code is in HLSL.
            // For OpenGL, the engine will convert this into GLSL under the hood.
            shader_create_info.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
            // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
            shader_create_info.UseCombinedTextureSamplers = true;
            shader_create_info.pShaderSourceStreamFactory = pso_info.shader_source_factory;

            Diligent::ShaderMacroHelper macros;
            macros.AddShaderMacro("NUM_TEXTURES", num_textures);
            macros.AddShaderMacro("NUM_SAMPLERS", num_samplers);
            shader_create_info.Macros = macros;
            
            // Vulkan and DirectX require DXC shader compiler.
            // Metal uses the builtin glslang compiler.
            #if PLATFORM_MACOS || PLATFORM_IOS || PLATFORM_TVOS
                const Diligent::SHADER_COMPILER compiler = Diligent::SHADER_COMPILER_DEFAULT;
            #else
                const Diligent::SHADER_COMPILER compiler = Diligent::SHADER_COMPILER_DXC;
            #endif
            shader_create_info.ShaderCompiler = compiler;

            // Create a vertex shader
            Diligent::RefCntAutoPtr<Diligent::IShader> vertex_shader;
            {
                shader_create_info.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
                shader_create_info.EntryPoint = pso_info.vertex_shader.entry_point.c_str();
                shader_create_info.Desc.Name = pso_info.vertex_shader.name.c_str();
                shader_create_info.FilePath = pso_info.vertex_shader.path.c_str();

                device->CreateShader(shader_create_info, &vertex_shader);
            }

            // Create a pixel shader
            Diligent::RefCntAutoPtr<Diligent::IShader> pixel_shader;
            {
                shader_create_info.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
                shader_create_info.EntryPoint = pso_info.vertex_shader.entry_point.c_str();
                shader_create_info.Desc.Name = pso_info.pixel_shader.name.c_str();
                shader_create_info.FilePath = pso_info.pixel_shader.path.c_str();

                device->CreateShader(shader_create_info, &pixel_shader);
            }

            pipeline_pso_info.pVS = vertex_shader;
            pipeline_pso_info.pPS = pixel_shader;

            Diligent::LayoutElement layout_elements[] =
            {
                Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
                Diligent::LayoutElement{1, 0, 3, Diligent::VT_FLOAT32, false},
                Diligent::LayoutElement{2, 0, 2, Diligent::VT_FLOAT32, false}
            };
            pipeline_pso_info.GraphicsPipeline.InputLayout.LayoutElements = layout_elements;
            pipeline_pso_info.GraphicsPipeline.InputLayout.NumElements = _countof(layout_elements);

            // Define variable type that will be used by default
            pipeline_pso_info.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
            pipeline_pso_info.PSODesc.ResourceLayout.DefaultVariableMergeStages = Diligent::SHADER_TYPE_VERTEX | Diligent::SHADER_TYPE_PIXEL;

            // Shader variables should typically be mutable, which means they are expected
            // to change on a per-instance basis
            pipeline_pso_info.PSODesc.ResourceLayout.Variables = pso_info.variables;
            pipeline_pso_info.PSODesc.ResourceLayout.NumVariables = pso_info.nb_variables;

            // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
            pipeline_pso_info.PSODesc.ResourceLayout.ImmutableSamplers = pso_info.immutable_samplers;
            pipeline_pso_info.PSODesc.ResourceLayout.NumImmutableSamplers = pso_info.nb_immutable_samplers;

            Diligent::RefCntAutoPtr<Diligent::IPipelineState> pso;
            device->CreateGraphicsPipelineState(pipeline_pso_info, &pso);

            return pso;
        }

        Diligent::RefCntAutoPtr<Diligent::ITexture> load_texture(
            Diligent::IRenderDevice* device, 
            const std::string& texture_path
        )
        {
            assert(device);
            
            Diligent::TextureLoadInfo texture_load_info;
            texture_load_info.IsSRGB = true;

            Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
            Diligent::CreateTextureFromFile(texture_path.c_str(), texture_load_info, device, &texture);

            return texture;
        }
    }
}
