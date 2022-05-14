#include "graphics_engine.hpp"

namespace engine
{
    // MARK: - Public

    GraphicsEngine::GraphicsEngine():
        last_time_(timer_.GetElapsedTime())
    {}

    void GraphicsEngine::initialize(const Diligent::NativeWindow* window)
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
            if (!create_device_and_swap_chain_metal_(window))
                assert(false);
        #else
            assert(false);
        #endif

        assert(device_);
        assert(context_);
        assert(swap_chain_);

        auto mj_texture = object::load_texture(device_, "./engine/assets/mj.jpg");
        auto wood_texture = object::load_texture(device_, "./engine/assets/wood.jpeg");

        std::vector<Diligent::StateTransitionDesc> barriers;

        Diligent::CreateUniformBuffer(device_, sizeof(Diligent::float4x4), "Vertex shader constants", &vertices_constants_);
        barriers.emplace_back(vertices_constants_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

        // MARK: - Cube

        create_cube_pso_();

        object::VERTEX_DATA cube_vertex_data;
        cube_vertex_data.positions = object::CUBE_POSITIONS;
        cube_vertex_data.normals = object::CUBE_NORMALS;
        cube_vertex_data.texcoords = object::CUBE_TEXTCOORDS;

        cube_vertex_buffer_ = object::create_vertex_buffer(device_, cube_vertex_data, object::VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD);
        barriers.emplace_back(cube_vertex_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

        cube_index_buffer_ = object::create_index_buffer(device_, object::CUBE_INDICES);
        barriers.emplace_back(cube_index_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

        // MARK: - Plane

        create_plane_pso_();

        object::VERTEX_DATA plane_vertex_data;
        plane_vertex_data.positions = object::PLANE_POSITIONS;
        plane_vertex_data.normals = object::PLANE_NORMALS;
        plane_vertex_data.texcoords = object::PLANE_TEXTCOORDS;

        plane_vertex_buffer_ = object::create_vertex_buffer(device_, plane_vertex_data, object::VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD);
        barriers.emplace_back(plane_vertex_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

        plane_index_buffer_ = object::create_index_buffer(device_, object::PLANE_INDICES);
        barriers.emplace_back(plane_index_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
        
        // MARK: - Texture

        cube_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mj_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
        plane_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(wood_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));

        barriers.emplace_back(mj_texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
        barriers.emplace_back(wood_texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

        context_->TransitionResourceStates(static_cast<Diligent::Uint32>(barriers.size()), barriers.data());
    }

    //static double max_framerate = 120.0;

    void GraphicsEngine::update()
    {
        /*
        double current_time = timer_.GetElapsedTime();
        double diff = current_time - last_time_;

        if (diff >= 1.0 / max_framerate)
        {
            last_time_ = current_time;

            update_();
            render_();
            present_();
        }
        */

        update_();
        render_();
        present_();
    }

    void GraphicsEngine::stop()
    {
        quit_ = true;
    }

    void GraphicsEngine::resize(uint32_t width, uint32_t height)
    {
        swap_chain_->Resize(width, height);
    }

    // MARK: - Private

    void GraphicsEngine::update_()
    {
        // Camera is at (0, 0, -5) looking along the Z axis
        Diligent::float4x4 view = Diligent::float4x4::Translation(0.0f, 0.0f, 5.0f);

        // Get projection matrix adjusted to the current screen orientation
        auto projection = get_adjusted_projection_matrix_(45.0f, 0.1f, 100.f);

        // Compute world-view-projection matrix
        world_view_projection_matrix_ = view * projection;
    }

    void GraphicsEngine::render_cube_()
    {
        // Bind vertex and index buffers
        Diligent::IBuffer* buffers[] = { cube_vertex_buffer_ };
        context_->SetVertexBuffers(0, 1, buffers, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        context_->SetIndexBuffer(cube_index_buffer_, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        /// Set the pipeline state in the immediate context
        context_->SetPipelineState(cube_pso_);

        // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
        // makes sure that resources are transitioned to required states.
        context_->CommitShaderResources(cube_srb_, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Diligent::DrawIndexedAttribs draw_attributes(36, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL);
        context_->DrawIndexed(draw_attributes);
    }

    void GraphicsEngine::render_plane_()
    {
        // Bind vertex and index buffers
        Diligent::IBuffer* buffers[] = { plane_vertex_buffer_ };
        context_->SetVertexBuffers(0, 1, buffers, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        context_->SetIndexBuffer(plane_index_buffer_, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        /// Set the pipeline state in the immediate context
        context_->SetPipelineState(plane_pso_);

        // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
        // makes sure that resources are transitioned to required states.
        context_->CommitShaderResources(plane_srb_, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Diligent::DrawIndexedAttribs draw_attributes(6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL);
        context_->DrawIndexed(draw_attributes);
    }

    void GraphicsEngine::render_()
    {
        // Bind main back buffer
        auto* pRTV = swap_chain_->GetCurrentBackBufferRTV();
        auto* pDSV = swap_chain_->GetDepthBufferDSV();
        context_->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        /* ---- */

        /// Clear the back buffer
        const float clear_color[] = {0.01f, 0.01f, 0.01f, 1.0f};

        /// Before rendering anything on the screen we want to clear it:
        /// Let the engine perform required state transitions
        context_->ClearRenderTarget(pRTV, clear_color, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        context_->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        /* ---- */

        {
            // Map the buffer and write current world-view-projection matrix
            Diligent::MapHelper<Diligent::float4x4> constants(context_, vertices_constants_, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
            *constants = world_view_projection_matrix_.Transpose();
        }

        /* ---- */

        render_plane_();
        render_cube_();

        // Restore default render target
        context_->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    void GraphicsEngine::present_()
    {
        assert(swap_chain_);
        swap_chain_->Present(vsync_enabled_ ? 1 : 0);
    }

    Diligent::float4x4 GraphicsEngine::get_adjusted_projection_matrix_(float fov, float near, float far) const
    {
        const auto& swap_chain_desc = swap_chain_->GetDesc();

        float aspect_ratio = static_cast<float>(swap_chain_desc.Width) / static_cast<float>(swap_chain_desc.Height);
        float y_scale = 1.f / std::tan(fov / 2.f);
        float x_scale = y_scale / aspect_ratio;

        Diligent::float4x4 projection;
        projection._11 = x_scale;
        projection._22 = y_scale;
        projection.SetNearFarClipPlanes(near, far, device_->GetDeviceInfo().IsGLDevice());

        return projection;
    }

    bool GraphicsEngine::create_device_and_swap_chain_metal_(const Diligent::NativeWindow* window)
    {
        auto *engine_factory = Diligent::GetEngineFactoryMtl();

        assert(engine_factory);

        Diligent::EngineMtlCreateInfo diligent_engine_create_info;
        diligent_engine_create_info.EnableValidation = false;
        diligent_engine_create_info.Features.ShaderFloat16 = Diligent::DEVICE_FEATURE_STATE_OPTIONAL;
        diligent_engine_create_info.Features.UniformBuffer16BitAccess = Diligent::DEVICE_FEATURE_STATE_OPTIONAL;

        engine_factory->CreateDeviceAndContextsMtl(diligent_engine_create_info, &device_, &context_);
        if (device_ == nullptr || context_ == nullptr)
            return false;

        create_swap_chain_metal_(window);

        return true;
    }

    void GraphicsEngine::create_swap_chain_metal_(const Diligent::NativeWindow* window)
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
        assert(swap_chain_);

        // QueryInterface() adds strong references, so we have to release them
        context->Release();
        device->Release();
    }

    void GraphicsEngine::create_cube_pso_()
    {
        auto *engine_factory = Diligent::GetEngineFactoryMtl();

        // Create a shader source stream factory to load shaders from files.
        Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> shader_source_factory;
        engine_factory->CreateDefaultShaderSourceStreamFactory(nullptr, &shader_source_factory);

        object::SHADER_INFO vertex_shader;
        vertex_shader.name = "Cube vertex shader";
        vertex_shader.path = "./engine/assets/shaders/cube/cube.vsh";

        object::SHADER_INFO pixel_shader;
        pixel_shader.name = "Cube pixel shader";
        pixel_shader.path = "./engine/assets/shaders/cube/cube.psh";

        object::PSO_INFO pso_info;
        pso_info.name = "Cube PSO";
        pso_info.rtv_format = swap_chain_->GetDesc().ColorBufferFormat;
        pso_info.dsv_format = swap_chain_->GetDesc().DepthBufferFormat;
        pso_info.shader_source_factory = shader_source_factory;
        pso_info.vertex_shader = vertex_shader;
        pso_info.pixel_shader = pixel_shader;
        pso_info.components = object::VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD;
        pso_info.cull_mode = Diligent::CULL_MODE_BACK;
        pso_info.depth_enable = false;
    
        Diligent::ShaderResourceVariableDesc variables[] = 
        {
            {Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
        };
        pso_info.variables = variables;
        pso_info.nb_variables = _countof(variables);

        Diligent::SamplerDesc sampler_linear_clamp_desc
        {
            Diligent::FILTER_TYPE_LINEAR, 
            Diligent::FILTER_TYPE_LINEAR, 
            Diligent::FILTER_TYPE_LINEAR, 

            Diligent::TEXTURE_ADDRESS_CLAMP, 
            Diligent::TEXTURE_ADDRESS_CLAMP, 
            Diligent::TEXTURE_ADDRESS_CLAMP
        };
        Diligent::ImmutableSamplerDesc immutable_samplers[] = 
        {
            {Diligent::SHADER_TYPE_PIXEL, "g_Texture", sampler_linear_clamp_desc}
        };
        pso_info.immutable_samplers = immutable_samplers;
        pso_info.nb_immutable_samplers = _countof(immutable_samplers);

        cube_pso_ = object::create_pipeline_state(device_, pso_info);

        // Since we did not explcitly specify the type for 'Constants' variable, default
        // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
        // change and are bound directly through the pipeline state object.
        cube_pso_->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(vertices_constants_);

        // Since we are using mutable variable, we must create a shader resource binding object
        // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
        cube_pso_->CreateShaderResourceBinding(&cube_srb_, true);
    }

    void GraphicsEngine::create_plane_pso_()
    {
        auto *engine_factory = Diligent::GetEngineFactoryMtl();

        // Create a shader source stream factory to load shaders from files.
        Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> shader_source_factory;
        engine_factory->CreateDefaultShaderSourceStreamFactory(nullptr, &shader_source_factory);

        object::SHADER_INFO vertex_shader;
        vertex_shader.name = "Plane vertex shader";
        vertex_shader.path = "./engine/assets/shaders/cube/cube.vsh";

        object::SHADER_INFO pixel_shader;
        pixel_shader.name = "Plane pixel shader";
        pixel_shader.path = "./engine/assets/shaders/cube/cube.psh";

        object::PSO_INFO pso_info;
        pso_info.name = "Plane PSO";
        pso_info.rtv_format = swap_chain_->GetDesc().ColorBufferFormat;
        pso_info.dsv_format = swap_chain_->GetDesc().DepthBufferFormat;
        pso_info.shader_source_factory = shader_source_factory;
        pso_info.vertex_shader = vertex_shader;
        pso_info.pixel_shader = pixel_shader;
        pso_info.components = object::VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD;
        pso_info.cull_mode = Diligent::CULL_MODE_BACK;
        pso_info.depth_enable = false;
    
        Diligent::ShaderResourceVariableDesc variables[] = 
        {
            {Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
        };
        pso_info.variables = variables;
        pso_info.nb_variables = _countof(variables);

        Diligent::SamplerDesc sampler_linear_clamp_desc
        {
            Diligent::FILTER_TYPE_LINEAR, 
            Diligent::FILTER_TYPE_LINEAR, 
            Diligent::FILTER_TYPE_LINEAR, 

            Diligent::TEXTURE_ADDRESS_CLAMP, 
            Diligent::TEXTURE_ADDRESS_CLAMP, 
            Diligent::TEXTURE_ADDRESS_CLAMP
        };
        Diligent::ImmutableSamplerDesc immutable_samplers[] = 
        {
            {Diligent::SHADER_TYPE_PIXEL, "g_Texture", sampler_linear_clamp_desc}
        };
        pso_info.immutable_samplers = immutable_samplers;
        pso_info.nb_immutable_samplers = _countof(immutable_samplers);

        plane_pso_ = object::create_pipeline_state(device_, pso_info);

        // Since we did not explcitly specify the type for 'Constants' variable, default
        // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
        // change and are bound directly through the pipeline state object.
        plane_pso_->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(vertices_constants_);

        // Since we are using mutable variable, we must create a shader resource binding object
        // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
        plane_pso_->CreateShaderResourceBinding(&plane_srb_, true);
    }
}
