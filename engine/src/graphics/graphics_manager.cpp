#include "graphics_manager.hpp"

namespace engine
{
    namespace graphics
    {
        /// MARK: - Public methods

        GraphicsManager::GraphicsManager()
        {}

        void GraphicsManager::initialize(const Diligent::NativeWindow* window)
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

            std::vector<Diligent::StateTransitionDesc> barriers;

            Diligent::CreateUniformBuffer(device_, sizeof(Diligent::float4x4), "Vertex shader constants", &vertices_constants_);
            barriers.emplace_back(vertices_constants_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

            // MARK: - Chunk

            create_chunk_pso_();

            object::VERTEX_DATA chunk_vertex_data = object::generate_random_chunk();

            chunk_vertex_buffer_ = object::create_vertex_buffer(device_, chunk_vertex_data, object::VERTEX_COMPONENT_FLAG_POSITION_TEXCOORD);
            barriers.emplace_back(chunk_vertex_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

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

            // MARK: - Sphere

            create_sphere_pso_();

            auto sphere = object::sphere::UVSphere(1.0, 30.0, 30.0);

            object::VERTEX_DATA sphere_vertex_data;
            sphere_vertex_data.positions = sphere.vertices_;
            sphere_vertex_data.normals = sphere.normals_;
            sphere_vertex_data.texcoords = sphere.textcoords_;

            sphere_vertex_buffer_ = object::create_vertex_buffer(device_, sphere_vertex_data, object::VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD);
            barriers.emplace_back(sphere_vertex_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

            sphere_index_buffer_ = object::create_index_buffer(device_, sphere.indices_);
            barriers.emplace_back(sphere_index_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

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

            auto dirt_texture = object::load_texture(device_, "./engine/assets/dirt.jpg");
            auto wood_texture = object::load_texture(device_, "./engine/assets/wood.jpeg");
            auto mj_texture = object::load_texture(device_, "./engine/assets/mj.jpg");

            chunk_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(dirt_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
            cube_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(dirt_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
            sphere_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mj_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
            plane_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(wood_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));

            barriers.emplace_back(dirt_texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
            barriers.emplace_back(wood_texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

            context_->TransitionResourceStates(static_cast<Diligent::Uint32>(barriers.size()), barriers.data());
        }

        void GraphicsManager::update(double dt)
        {
            update_(dt);
            render_();
            present_();
        }

        void GraphicsManager::shutdown()
        {
            // TODO: DO SOMETHING
        }

        void GraphicsManager::resize(uint32_t width, uint32_t height)
        {
            assert(swap_chain_);
            
            swap_chain_->Resize(width, height);
        }

        void GraphicsManager::set_fov(double fov)
        {
            fov_ = fov;
        }

        void GraphicsManager::set_camera_view(Diligent::float4x4 camera_view)
        {
            camera_view_ = camera_view;
        }

        /// MARK: - Private methods

        void GraphicsManager::update_(double dt)
        {
            // Get projection matrix adjusted to the current screen orientation
            auto projection = get_adjusted_projection_matrix_(fov_, 0.1f, 100.f);

            // Compute world-view-projection matrix
            world_view_projection_matrix_ = camera_view_ * projection;
        }

        void GraphicsManager::render_chunk_()
        {
            Diligent::IBuffer* buffers[] = { chunk_vertex_buffer_ };
            context_->SetVertexBuffers(0, 1, buffers, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);

            /// Set the pipeline state in the immediate context
            context_->SetPipelineState(chunk_pso_);

            // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
            // makes sure that resources are transitioned to required states.
            context_->CommitShaderResources(chunk_srb_, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            Diligent::DrawAttribs draw_attributes(chunk_vertex_buffer_->GetDesc().Size / sizeof(Diligent::float3), Diligent::DRAW_FLAG_VERIFY_ALL);
            context_->Draw(draw_attributes);
        }

        void GraphicsManager::render_cube_()
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

            Diligent::DrawIndexedAttribs draw_attributes(cube_index_buffer_->GetDesc().Size / sizeof(Diligent::Uint32), Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL);
            context_->DrawIndexed(draw_attributes);
        }

        void GraphicsManager::render_sphere_()
        {
            // Bind vertex and index buffers
            Diligent::IBuffer* buffers[] = { sphere_vertex_buffer_ };
            context_->SetVertexBuffers(0, 1, buffers, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
            context_->SetIndexBuffer(sphere_index_buffer_, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            /// Set the pipeline state in the immediate context
            context_->SetPipelineState(sphere_pso_);

            // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
            // makes sure that resources are transitioned to required states.
            context_->CommitShaderResources(sphere_srb_, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            Diligent::DrawIndexedAttribs draw_attributes(sphere_index_buffer_->GetDesc().Size / sizeof(Diligent::Uint32), Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL);
            context_->DrawIndexed(draw_attributes);
        }

        void GraphicsManager::render_plane_()
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

            Diligent::DrawIndexedAttribs draw_attributes(plane_index_buffer_->GetDesc().Size / sizeof(Diligent::Uint32), Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL);
            context_->DrawIndexed(draw_attributes);
        }

        void GraphicsManager::render_()
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
            
            //render_chunk_();
            render_plane_();
            //render_cube_();
            render_sphere_();

            // Restore default render target
            context_->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        void GraphicsManager::present_()
        {
            assert(swap_chain_);
            
            swap_chain_->Present(vsync_enabled_ ? 1 : 0);
        }

        Diligent::float4x4 GraphicsManager::get_adjusted_projection_matrix_(float fov, float near, float far) const
        {
            const auto& swap_chain_desc = swap_chain_->GetDesc();

            float aspect_ratio = static_cast<float>(swap_chain_desc.Width) / static_cast<float>(swap_chain_desc.Height);

            return Diligent::float4x4::Projection(
                utils::degrees_to_radians(fov),
                aspect_ratio, 
                near, far, 
                device_->GetDeviceInfo().IsGLDevice()
            );
        }

        bool GraphicsManager::create_device_and_swap_chain_metal_(const Diligent::NativeWindow* window)
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

        void GraphicsManager::create_swap_chain_metal_(const Diligent::NativeWindow* window)
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

        void GraphicsManager::create_chunk_pso_()
        {
            auto *engine_factory = Diligent::GetEngineFactoryMtl();

            // Create a shader source stream factory to load shaders from files.
            Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> shader_source_factory;
            engine_factory->CreateDefaultShaderSourceStreamFactory(nullptr, &shader_source_factory);

            object::SHADER_INFO vertex_shader;
            vertex_shader.name = "Chunk vertex shader";
            vertex_shader.path = "./engine/assets/shaders/chunk/chunk.vsh";

            object::SHADER_INFO pixel_shader;
            pixel_shader.name = "Chunk pixel shader";
            pixel_shader.path = "./engine/assets/shaders/chunk/chunk.psh";

            object::PSO_INFO pso_info;
            pso_info.name = "Chunk PSO";
            pso_info.rtv_format = swap_chain_->GetDesc().ColorBufferFormat;
            pso_info.dsv_format = swap_chain_->GetDesc().DepthBufferFormat;
            pso_info.shader_source_factory = shader_source_factory;
            pso_info.vertex_shader = vertex_shader;
            pso_info.pixel_shader = pixel_shader;
            pso_info.components = object::VERTEX_COMPONENT_FLAG_POSITION_TEXCOORD;
            pso_info.cull_mode = Diligent::CULL_MODE_BACK;
            pso_info.depth_enable = true;

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

            chunk_pso_ = object::create_pipeline_state(device_, pso_info);

            // Since we did not explcitly specify the type for 'Constants' variable, default
            // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
            // change and are bound directly through the pipeline state object.
            chunk_pso_->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(vertices_constants_);

            // Since we are using mutable variable, we must create a shader resource binding object
            // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
            chunk_pso_->CreateShaderResourceBinding(&chunk_srb_, true);
        }

        void GraphicsManager::create_cube_pso_()
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
            pso_info.depth_enable = true;
        
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

        void GraphicsManager::create_sphere_pso_()
        {
            auto *engine_factory = Diligent::GetEngineFactoryMtl();

            // Create a shader source stream factory to load shaders from files.
            Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> shader_source_factory;
            engine_factory->CreateDefaultShaderSourceStreamFactory(nullptr, &shader_source_factory);

            object::SHADER_INFO vertex_shader;
            vertex_shader.name = "Sphere vertex shader";
            vertex_shader.path = "./engine/assets/shaders/cube/cube.vsh";

            object::SHADER_INFO pixel_shader;
            pixel_shader.name = "Sphere pixel shader";
            pixel_shader.path = "./engine/assets/shaders/cube/cube.psh";

            object::PSO_INFO pso_info;
            pso_info.name = "Sphere PSO";
            pso_info.rtv_format = swap_chain_->GetDesc().ColorBufferFormat;
            pso_info.dsv_format = swap_chain_->GetDesc().DepthBufferFormat;
            pso_info.shader_source_factory = shader_source_factory;
            pso_info.vertex_shader = vertex_shader;
            pso_info.pixel_shader = pixel_shader;
            pso_info.components = object::VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD;
            pso_info.cull_mode = Diligent::CULL_MODE_BACK;
            pso_info.depth_enable = true;
        
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

            sphere_pso_ = object::create_pipeline_state(device_, pso_info);

            // Since we did not explcitly specify the type for 'Constants' variable, default
            // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
            // change and are bound directly through the pipeline state object.
            sphere_pso_->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(vertices_constants_);

            // Since we are using mutable variable, we must create a shader resource binding object
            // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
            sphere_pso_->CreateShaderResourceBinding(&sphere_srb_, true);
        }

        void GraphicsManager::create_plane_pso_()
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
            pso_info.depth_enable = true;
        
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
}
