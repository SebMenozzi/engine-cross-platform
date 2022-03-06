#include "PlatformDefinitions.h"
#include "Errors.hpp"

#if METAL_SUPPORTED
#include "EngineFactoryMtl.h"
#endif

#include "GraphicsEngine.hpp"

namespace Diligent
{
    // MARK: - Public

    void GraphicsEngine::initialize_diligent_engine(const NativeWindow* window)
    {
        // Initialize the swap chain descriptor
        #if PLATFORM_MACOS
        // We need at least 3 buffers in Metal to avoid massive
        // performance degradation in full screen mode.
        // https://github.com/KhronosGroup/MoltenVK/issues/808
        swap_chain_desc_.BufferCount = 3;
        #endif
        swap_chain_desc_.Width = 1;
        swap_chain_desc_.Height = 1;

        #if PLATFORM_MACOS
        if (!create_device_and_swap_chain_metal(window))
            assert(false);
        #endif

        create_pipeline_state();
        create_vertex_buffer();
        create_index_buffer();
    }

    void GraphicsEngine::start()
    {
        double framerate = 0.001;

        double last_time = timer_.GetElapsedTime();

        while (!quit_)
        {
            double current_time = timer_.GetElapsedTime();

            if (current_time - last_time >= 1.0 / framerate)
            {
                last_time = current_time;

                update();
                render();
                present();
            }
        }
    }

    void GraphicsEngine::stop()
    {
        quit_ = true;
    }

    // MARK: - Private

    void GraphicsEngine::update()
    {
        auto time = timer_.GetElapsedTime();

        // Apply rotation
        float4x4 transform = float4x4::RotationY(static_cast<float>(time) * 1.0f) * float4x4::RotationX(-PI_F * 0.1f);

        // Camera is at (0, 0, -5) looking along the Z axis
        float4x4 view = float4x4::Translation(0.f, 0.0f, 5.0f);

        // Get projection matrix adjusted to the current screen orientation
        auto projection = get_adjusted_projection_matrix(PI_F / 4.0f, 0.1f, 100.f);

        // Compute world-view-projection matrix
        world_view_projection_matrix_ = transform * view * projection;
    }

    void GraphicsEngine::render()
    {
        context_->SetRenderTargets(0, nullptr, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        auto* pRTV = swap_chain_->GetCurrentBackBufferRTV();
        auto* pDSV = swap_chain_->GetDepthBufferDSV();
        context_->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        /* ---- */

        /// Clear the back buffer
        const float clear_color[] = {0.0f, 0.0f, 0.0f, 1.0f};

        /// Before rendering anything on the screen we want to clear it:
        /// Let the engine perform required state transitions
        context_->ClearRenderTarget(pRTV, clear_color, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        context_->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        /* ---- */

        {
            // Map the buffer and write current world-view-projection matrix
            MapHelper<float4x4> constants(context_, vertices_constants_, MAP_WRITE, MAP_FLAG_DISCARD);
            *constants = world_view_projection_matrix_.Transpose();
        }

        // Bind vertex and index buffers
        const Uint64 offset = 0;
        IBuffer* buffers[] = { cube_vertex_buffer_ };
        context_->SetVertexBuffers(0, 1, buffers, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        context_->SetIndexBuffer(cube_index_buffer_, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        /// Set the pipeline state in the immediate context
        context_->SetPipelineState(pso_);

        // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
        // makes sure that resources are transitioned to required states.
        context_->CommitShaderResources(srb_, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        /// Finally, we invoke the draw command that renders our 3 vertices:
        DrawIndexedAttribs draw_attributes;
        draw_attributes.IndexType = VT_UINT32; // Index type
        draw_attributes.NumIndices = 36;
        // Verify the state of vertex and index buffers
        draw_attributes.Flags = DRAW_FLAG_VERIFY_ALL;
        context_->DrawIndexed(draw_attributes);

        /* ---- */

        // Restore default render target in case the sample has changed it
        context_->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    void GraphicsEngine::present()
    {
        if (!swap_chain_)
            return;

        swap_chain_->Present(vsync_enabled_ ? 1 : 0);
    }

    float4x4 GraphicsEngine::get_adjusted_projection_matrix(float fov, float near, float far) const
    {
        const auto& swap_chain_desc = swap_chain_->GetDesc();

        float aspect_ratio = static_cast<float>(swap_chain_desc.Width) / static_cast<float>(swap_chain_desc.Height);
        float y_scale = 1.f / std::tan(fov / 2.f);
        float x_scale = y_scale / aspect_ratio;

        float4x4 projection;
        projection._11 = x_scale;
        projection._22 = y_scale;
        projection.SetNearFarClipPlanes(near, far, device_->GetDeviceInfo().IsGLDevice());

        return projection;
    }

    bool GraphicsEngine::create_device_and_swap_chain_metal(const NativeWindow* window)
    {
        auto *engine_factory = Diligent::GetEngineFactoryMtl();

        VERIFY_EXPR(engine_factory);

        Diligent::EngineMtlCreateInfo diligent_engine_create_info;
        diligent_engine_create_info.EnableValidation = false;
        diligent_engine_create_info.Features.ShaderFloat16 = Diligent::DEVICE_FEATURE_STATE_OPTIONAL;
        diligent_engine_create_info.Features.UniformBuffer16BitAccess = Diligent::DEVICE_FEATURE_STATE_OPTIONAL;

        engine_factory->CreateDeviceAndContextsMtl(diligent_engine_create_info, &device_, &context_);
        if (device_ == nullptr || context_ == nullptr)
            return false;

        create_swap_chain_metal(window);

        return true;
    }

    void GraphicsEngine::create_swap_chain_metal(const NativeWindow* window)
    {
        // Get a pointer to the Metal engine factory
        auto *engine_factory = Diligent::GetEngineFactoryMtl();

        // Query the interface for the render device in case it was proxified
        Diligent::IRenderDevice *device = nullptr;
        device_->QueryInterface(Diligent::IID_RenderDevice, reinterpret_cast<Diligent::IObject **>(&device));
        assert(device);

        // Query the interface for the device context in case it was proxified
        Diligent::IDeviceContext *context = nullptr;
        context_->QueryInterface(Diligent::IID_DeviceContext, reinterpret_cast<Diligent::IObject **>(&context));
        assert(context);

        // Create the swap chain
        engine_factory->CreateSwapChainMtl(device_, context_, swap_chain_desc_, *window, &swap_chain_);
        VERIFY_EXPR(swap_chain_);

        // QueryInterface() adds strong references, so we have to release them
        context->Release();
        device->Release();
    }

    std::optional<Buffer> GraphicsEngine::read_file(const std::filesystem::path &path)
    {
        // Verify that the provided file path exists on the filesystem
        std::error_code error_code;
        if (!std::filesystem::exists(path, error_code)) {
            return {};
        }

        // Open the file with read permissions
        std::ifstream stream(path.native(), std::ios::in | std::ios::binary | std::ios::ate);
        if (!stream) {
            return {};
        }

        // Resize the output data to accomodate the file's content
        Buffer bytes;
        bytes.resize(stream.tellg());

        // Read the whole file
        stream.seekg(0);
        stream.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
        if (stream.fail()) {
            return {};
        }

        // Return the resulting file's content
        return bytes;
    }

    void GraphicsEngine::create_pipeline_state()
    {
        // Pipeline state object encompasses configuration of all GPU stages

        GraphicsPipelineStateCreateInfo pso_info;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        pso_info.PSODesc.Name = "Cube PSO";

        // This is a graphics pipeline
        pso_info.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        pso_info.GraphicsPipeline.NumRenderTargets = 1;
        // Set render target format which is the format of the swap chain's color buffer
        pso_info.GraphicsPipeline.RTVFormats[0] = swap_chain_->GetDesc().ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        pso_info.GraphicsPipeline.DSVFormat = swap_chain_->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        pso_info.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        pso_info.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
        // Disable depth testing
        pso_info.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
        // clang-format on

        ShaderCreateInfo shader_info;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood.
        shader_info.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        shader_info.UseCombinedTextureSamplers = true;

        // Create a vertex shader
        const auto vertex_shader_file = read_file("/Users/sebasteinmenozzi/Documents/engine-cross-platform/engine/assets/shaders/Cube.vsh");
        assert(vertex_shader_file.has_value());

        //auto *engine_factory = Diligent::GetEngineFactoryMtl();

        //RefCntAutoPtr<IShaderSourceInputStreamFactory> shader_source_factory;
        //engine_factory->CreateDefaultShaderSourceStreamFactory(nullptr, &shader_source_factory);
        //shader_info.pShaderSourceStreamFactory = shader_source_factory;

        RefCntAutoPtr<IShader> vertex_shader;
        {
            shader_info.Desc.ShaderType = SHADER_TYPE_VERTEX;
            shader_info.EntryPoint = "main";
            shader_info.Desc.Name = "Cube vertex shader";
            //shader_info.FilePath = "/Users/sebasteinmenozzi/Documents/engine-cross-platform/engine/assets/shaders/Cube.vsh";
            shader_info.Source = reinterpret_cast<const char *>(vertex_shader_file->data());
            shader_info.SourceLength = vertex_shader_file->size();
            device_->CreateShader(shader_info, &vertex_shader);

            // Create dynamic uniform buffer that will store our transformation matrix
            // Dynamic buffers can be frequently updated by the CPU
            BufferDesc buffer_desc;
            buffer_desc.Name = "Vertex shader constants";
            buffer_desc.Size = sizeof(float4x4);
            buffer_desc.Usage = USAGE_DYNAMIC;
            buffer_desc.BindFlags = BIND_UNIFORM_BUFFER;
            buffer_desc.CPUAccessFlags = CPU_ACCESS_WRITE;
            device_->CreateBuffer(buffer_desc, nullptr, &vertices_constants_);
        }

        // Create a pixel shader

        const auto pixel_shader_file = read_file("/Users/sebasteinmenozzi/Documents/engine-cross-platform/engine/assets/shaders/Cube.psh");
        assert(pixel_shader_file.has_value());

        RefCntAutoPtr<IShader> pixel_shader;
        {
            shader_info.Desc.ShaderType = SHADER_TYPE_PIXEL;
            shader_info.EntryPoint = "main";
            shader_info.Desc.Name = "Cube pixel shader";
            shader_info.Source = reinterpret_cast<const char *>(pixel_shader_file->data());
            shader_info.SourceLength = pixel_shader_file->size();
            //shader_info.FilePath = "/Users/sebasteinmenozzi/Documents/engine-cross-platform/engine/assets/shaders/Cube.psh";
            device_->CreateShader(shader_info, &pixel_shader);
        }

        // clang-format off
        // Define vertex shader input layout
        LayoutElement layout_elements[] =
        {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - vertex color
            LayoutElement{1, 0, 4, VT_FLOAT32, False}
        };
        // clang-format on
        pso_info.GraphicsPipeline.InputLayout.LayoutElements = layout_elements;
        pso_info.GraphicsPipeline.InputLayout.NumElements = _countof(layout_elements);

        // Finally, create the pipeline state
        pso_info.pVS = vertex_shader;
        pso_info.pPS = pixel_shader;

        // Define variable type that will be used by default
        pso_info.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        device_->CreateGraphicsPipelineState(pso_info, &pso_);

        // Since we did not explcitly specify the type for 'Constants' variable, default
        // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
        // change and are bound directly through the pipeline state object.
        pso_->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(vertices_constants_);

        // Create a shader resource binding object and bind all static resources in it
        pso_->CreateShaderResourceBinding(&srb_, true);
    }

    void GraphicsEngine::create_vertex_buffer()
    {
        // Layout of this structure matches the one we defined in the pipeline state
        struct Vertex
        {
            float3 pos;
            float4 color;
        };

        // Cube vertices

        //      (-1,+1,+1)________________(+1,+1,+1)
        //               /|              /|
        //              / |             / |
        //             /  |            /  |
        //            /   |           /   |
        //(-1,-1,+1) /____|__________/(+1,-1,+1)
        //           |    |__________|____|
        //           |   /(-1,+1,-1) |    /(+1,+1,-1)
        //           |  /            |   /
        //           | /             |  /
        //           |/              | /
        //           /_______________|/
        //        (-1,-1,-1)       (+1,-1,-1)
        //

        // clang-format off
        Vertex vertices[8] =
        {
            {float3(-1,-1,-1), float4(1,0,0,1)},
            {float3(-1,+1,-1), float4(0,1,0,1)},
            {float3(+1,+1,-1), float4(0,0,1,1)},
            {float3(+1,-1,-1), float4(1,1,1,1)},

            {float3(-1,-1,+1), float4(1,1,0,1)},
            {float3(-1,+1,+1), float4(0,1,1,1)},
            {float3(+1,+1,+1), float4(1,0,1,1)},
            {float3(+1,-1,+1), float4(0.2f,0.2f,0.2f,1)},
        };
        // clang-format on

        // Create a vertex buffer that stores cube vertices
        BufferDesc vertex_buffer_desc;
        vertex_buffer_desc.Name = "Cube vertex buffer";
        vertex_buffer_desc.Usage = USAGE_IMMUTABLE;
        vertex_buffer_desc.BindFlags = BIND_VERTEX_BUFFER;
        vertex_buffer_desc.Size = sizeof(vertices);

        BufferData vertex_data;
        vertex_data.pData = vertices;
        vertex_data.DataSize = sizeof(vertices);

        device_->CreateBuffer(vertex_buffer_desc, &vertex_data, &cube_vertex_buffer_);
    }

    void GraphicsEngine::create_index_buffer()
    {
        // clang-format off
        Uint32 indices[] =
        {
            2,0,1, 2,3,0,
            4,6,5, 4,7,6,
            0,7,4, 0,3,7,
            1,0,4, 1,4,5,
            1,5,2, 5,6,2,
            3,6,7, 3,2,6
        };
        // clang-format on

        BufferDesc index_buffer_desc;
        index_buffer_desc.Name = "Cube index buffer";
        index_buffer_desc.Usage = USAGE_IMMUTABLE;
        index_buffer_desc.BindFlags = BIND_INDEX_BUFFER;
        index_buffer_desc.Size = sizeof(indices);

        BufferData index_data;
        index_data.pData = indices;
        index_data.DataSize = sizeof(indices);

        device_->CreateBuffer(index_buffer_desc, &index_data, &cube_index_buffer_);
    }
}
