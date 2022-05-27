
#include "object.hpp"

namespace engine
{
    namespace object
    {
        Diligent::RefCntAutoPtr<Diligent::ITexture> load_texture(
            Diligent::IRenderDevice* device, 
            const std::string& texture_path
        )
        {
            Diligent::TextureLoadInfo texture_load_info;
            texture_load_info.IsSRGB = true;

            Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
            CreateTextureFromFile(texture_path.c_str(), texture_load_info, device, &texture);

            return texture;
        }

        Diligent::RefCntAutoPtr<Diligent::IBuffer> create_vertex_buffer(
            Diligent::IRenderDevice* device,
            VERTEX_DATA vertex_data,
            VERTEX_COMPONENT_FLAGS components,
            Diligent::BIND_FLAGS bind_flags,
            Diligent::BUFFER_MODE mode
        )
        {
            assert(components != VERTEX_COMPONENT_FLAG_NONE);
            
            const Diligent::Uint32 total_vertex_components =
                ((components & VERTEX_COMPONENT_FLAG_POSITION) ? 3 : 0) +
                ((components & VERTEX_COMPONENT_FLAG_NORMAL) ? 3 : 0) +
                ((components & VERTEX_COMPONENT_FLAG_TEXCOORD) ? 2 : 0);

            const Diligent::Uint32 nb_vertices = Diligent::Uint32(vertex_data.positions.size());

            std::vector<float> data(total_vertex_components * nb_vertices);

            auto it = data.begin();

            for (Diligent::Uint32 v = 0; v < nb_vertices; ++v)
            {
                if (components & VERTEX_COMPONENT_FLAG_POSITION)
                {
                    const auto& position{vertex_data.positions[v]};
                    *(it++) = position.x;
                    *(it++) = position.y;
                    *(it++) = position.z;
                }
                if (components & VERTEX_COMPONENT_FLAG_NORMAL)
                {
                    const auto& normal{vertex_data.normals[v]};
                    *(it++) = normal.x;
                    *(it++) = normal.y;
                    *(it++) = normal.z;
                }
                if (components & VERTEX_COMPONENT_FLAG_TEXCOORD)
                {
                    const auto& texcoord{vertex_data.texcoords[v]};
                    *(it++) = texcoord.x;
                    *(it++) = texcoord.y;
                }
            }

            assert(it == data.end());

            // Create a vertex buffer that stores cube vertices
            Diligent::BufferDesc vertex_buffer_desc;
            vertex_buffer_desc.Name = "Vertex buffer";
            vertex_buffer_desc.Usage = Diligent::USAGE_IMMUTABLE;
            vertex_buffer_desc.BindFlags = bind_flags;
            vertex_buffer_desc.Size = static_cast<Diligent::Uint64>(data.size() * sizeof(float));
            vertex_buffer_desc.Mode = mode;

            if (mode != Diligent::BUFFER_MODE_UNDEFINED)
                vertex_buffer_desc.ElementByteStride = total_vertex_components * sizeof(float);

            Diligent::BufferData vertex_buffer_data;
            vertex_buffer_data.pData = data.data();
            vertex_buffer_data.DataSize = vertex_buffer_desc.Size;

            Diligent::RefCntAutoPtr<Diligent::IBuffer> vertex_buffer;
            device->CreateBuffer(vertex_buffer_desc, &vertex_buffer_data, &vertex_buffer);

            return vertex_buffer;
        }

        Diligent::RefCntAutoPtr<Diligent::IBuffer> create_index_buffer(
            Diligent::IRenderDevice* device, 
            const std::vector<Diligent::Uint32> indices,
            Diligent::BIND_FLAGS bind_flags, 
            Diligent::BUFFER_MODE mode
        )
        {
            const Diligent::Uint32 nb_indices = Diligent::Uint32(indices.size());

            Diligent::BufferDesc index_buffer_desc;
            index_buffer_desc.Name = "Index buffer";
            index_buffer_desc.Usage = Diligent::USAGE_IMMUTABLE;
            index_buffer_desc.BindFlags = bind_flags;
            index_buffer_desc.Size = nb_indices * sizeof(Diligent::Uint32);
            index_buffer_desc.Mode = mode;

            if (mode != Diligent::BUFFER_MODE_UNDEFINED)
                index_buffer_desc.ElementByteStride = sizeof(Diligent::Uint32);

            Diligent::BufferData index_data;
            index_data.pData = indices.data();
            index_data.DataSize = nb_indices * sizeof(Diligent::Uint32);

            Diligent::RefCntAutoPtr<Diligent::IBuffer> index_buffer;
            device->CreateBuffer(index_buffer_desc, &index_data, &index_buffer);

            return index_buffer;
        }

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> create_pipeline_state(
            Diligent::IRenderDevice* device,
            const PSO_INFO& pso_info
        )
        {
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
            pipeline_pso_info.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            // Cull back faces
            pipeline_pso_info.GraphicsPipeline.RasterizerDesc.CullMode = pso_info.cull_mode;
            // Enable depth testing
            pipeline_pso_info.GraphicsPipeline.DepthStencilDesc.DepthEnable = pso_info.depth_enable;

            Diligent::ShaderCreateInfo shader_create_info;
            // Tell the system that the shader source code is in HLSL.
            // For OpenGL, the engine will convert this into GLSL under the hood.
            shader_create_info.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
            // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
            shader_create_info.UseCombinedTextureSamplers = true;
            shader_create_info.pShaderSourceStreamFactory = pso_info.shader_source_factory;

            // Create a vertex shader
            Diligent::RefCntAutoPtr<Diligent::IShader> vertex_shader;
            {
                shader_create_info.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
                shader_create_info.EntryPoint = "main";
                shader_create_info.Desc.Name = pso_info.vertex_shader.name.c_str();
                shader_create_info.FilePath = pso_info.vertex_shader.path.c_str();

                device->CreateShader(shader_create_info, &vertex_shader);
            }

            // Create a pixel shader
            Diligent::RefCntAutoPtr<Diligent::IShader> pixel_shader;
            {
                shader_create_info.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
                shader_create_info.EntryPoint = "main";
                shader_create_info.Desc.Name = pso_info.pixel_shader.name.c_str();
                shader_create_info.FilePath = pso_info.pixel_shader.path.c_str();

                device->CreateShader(shader_create_info, &pixel_shader);
            }

            pipeline_pso_info.pVS = vertex_shader;
            pipeline_pso_info.pPS = pixel_shader;

            Diligent::InputLayoutDescX input_layout;
            Diligent::Uint32 nb_attributes = 0;
            if (pso_info.components & VERTEX_COMPONENT_FLAG_POSITION)
                input_layout.Add(nb_attributes++, 0, 3, Diligent::VT_FLOAT32, false);
            if (pso_info.components & VERTEX_COMPONENT_FLAG_NORMAL)
                input_layout.Add(nb_attributes++, 0, 3, Diligent::VT_FLOAT32, false);
            if (pso_info.components & VERTEX_COMPONENT_FLAG_TEXCOORD)
                input_layout.Add(nb_attributes++, 0, 2, Diligent::VT_FLOAT32, false);

            for (Diligent::Uint32 i = 0; i < pso_info.nb_layout_elements; ++i)
                input_layout.Add(pso_info.layout_elements[i]);

            pipeline_pso_info.GraphicsPipeline.InputLayout = input_layout;

            // Define variable type that will be used by default
            pipeline_pso_info.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

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
    }
}
