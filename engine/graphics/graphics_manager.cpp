#include "graphics_manager.hpp"
#include <cassert>

namespace engine
{
    namespace graphics
    {
        /// MARK: - Public methods

        GraphicsManager::GraphicsManager(const std::string& path)
            : assets_path_(path)
        {}

        void GraphicsManager::initialize(const Diligent::NativeWindow* window)
        {
            // Initialize the swap chain descriptor
            #if PLATFORM_MACOS || PLATFORM_IOS
                // We need at least 3 buffers in Metal to avoid massive
                // performance degradation in full screen mode.
                // https://github.com/KhronosGroup/MoltenVK/issues/808
                swap_chain_desc_.BufferCount = 3;
            #endif
            swap_chain_desc_.Width = 1;
            swap_chain_desc_.Height = 1;

            #if PLATFORM_MACOS || PLATFORM_IOS
                if (!create_device_and_swap_chain_metal_(window))
                    assert(false);
            #else
                assert(false);
            #endif

            assert(device_);
            assert(context_);
            assert(swap_chain_);
            
            // MARK: Post processing
            create_post_process_pso_();
            
            update_g_buffer_();

            {
                // Create constant buffers
                Diligent::BufferDesc buffer_desc;
                buffer_desc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
                buffer_desc.Usage = Diligent::USAGE_DEFAULT;
                buffer_desc.Size = sizeof(Diligent::GlobalConstants);
                buffer_desc.ImmediateContextMask = (Diligent::Uint64{1} << context_->GetDesc().ContextId);
                buffer_desc.Name = "Global constants";
                device_->CreateBuffer(buffer_desc, nullptr, &global_constants_);
            }

            // MARK: - Sphere

            create_sphere_pso_();

            auto sphere = object::sphere::UVSphere(1.0, 100.0, 100.0);

            VERTEX_DATA sphere_vertex_data;
            sphere_vertex_data.positions = sphere.vertices_;
            sphere_vertex_data.normals = sphere.normals_;
            sphere_vertex_data.texcoords = sphere.textcoords_;

            sphere_vertex_buffer_ = create_vertex_buffer(device_, sphere_vertex_data, VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD);
            sphere_index_buffer_ = create_index_buffer(device_, sphere.indices_);

            // MARK: - Plane

            create_plane_pso_();

            VERTEX_DATA plane_vertex_data;
            plane_vertex_data.positions = object::PLANE_POSITIONS;
            plane_vertex_data.normals = object::PLANE_NORMALS;
            plane_vertex_data.texcoords = object::PLANE_TEXTCOORDS;

            plane_vertex_buffer_ = create_vertex_buffer(device_, plane_vertex_data, VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD);
            plane_index_buffer_ = create_index_buffer(device_, object::PLANE_INDICES);

            // MARK: - Texture

            auto wood_texture = load_texture(device_, assets_path_ + "/wood.jpeg");
            auto mj_texture = load_texture(device_, assets_path_ + "/mj.jpg");

            sphere_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mj_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
            plane_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(wood_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
            
            sphere_srb_->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(global_constants_);
            plane_srb_->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(global_constants_);
            post_process_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(global_constants_);
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
    
        void GraphicsManager::update_g_buffer_()
        {
            assert(swap_chain_);
            assert(device_);
            
            const auto& swap_chain_desc = swap_chain_->GetDesc();
            
            {
                // Create window-size G-buffer textures.
                g_buffer_ = {};
                
                Diligent::TextureDesc texture_desc;
                texture_desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
                texture_desc.Width = swap_chain_desc.Width;
                texture_desc.Height = swap_chain_desc.Height;
                texture_desc.MipLevels = 1;

                texture_desc.Name = "GBuffer Color";
                texture_desc.BindFlags = Diligent::BIND_RENDER_TARGET | Diligent::BIND_SHADER_RESOURCE;
                texture_desc.Format = swap_chain_desc.ColorBufferFormat;
                device_->CreateTexture(texture_desc, nullptr, &g_buffer_.color_texture);
                
                texture_desc.Name = "GBuffer Depth";
                texture_desc.BindFlags = Diligent::BIND_DEPTH_STENCIL | Diligent::BIND_SHADER_RESOURCE;
                texture_desc.Format = swap_chain_desc.DepthBufferFormat;
                device_->CreateTexture(texture_desc, nullptr, &g_buffer_.depth_texture);
            }
            
            // Create post-processing SRB
            {
                // We need to release and create a new SRB that references new post process render target SRV
                post_process_srb_.Release();
                post_process_pso_->CreateShaderResourceBinding(&post_process_srb_, true);
                
                post_process_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_GBuffer_Color")->Set(g_buffer_.color_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
                post_process_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_GBuffer_Depth")->Set(g_buffer_.depth_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
            }
        }

        void GraphicsManager::resize(uint32_t width, uint32_t height)
        {
            assert(swap_chain_);
            assert(device_);
            
            swap_chain_->Resize(width, height);
            
            update_g_buffer_();
        }

        void GraphicsManager::set_fov(double fov)
        {
            fov_ = fov;
        }

        void GraphicsManager::set_camera_view(Diligent::float4x4 camera_view)
        {
            camera_view_ = camera_view;
        }

        void GraphicsManager::set_camera_position(Diligent::float3 camera_position)
        {
            camera_position_ = camera_position;
        }

        /// MARK: - Private methods

        void GraphicsManager::update_(double dt)
        {
            // Get projection matrix adjusted to the current screen orientation
            auto projection = get_adjusted_projection_matrix_(fov_, 0.1f, 100.f);

            // Compute camera-view-projection matrix
            camera_view_projection_ = camera_view_ * projection;
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
            
            const Diligent::Uint32 nb_elements = Diligent::Uint32(sphere_index_buffer_->GetDesc().Size);
            const Diligent::Uint32 element_size = sizeof(Diligent::Uint32);

            Diligent::DrawIndexedAttribs draw_attributes(nb_elements / element_size, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL);
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
            
            const Diligent::Uint32 nb_elements = Diligent::Uint32(plane_index_buffer_->GetDesc().Size);
            const Diligent::Uint32 element_size = sizeof(Diligent::Uint32);

            Diligent::DrawIndexedAttribs draw_attributes(nb_elements / element_size, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL);
            context_->DrawIndexed(draw_attributes);
        }

        void GraphicsManager::render_post_process_()
        {
            Diligent::ITextureView* pRTV = swap_chain_->GetCurrentBackBufferRTV();

            context_->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            
            {
                context_->SetPipelineState(post_process_pso_);
                context_->CommitShaderResources(post_process_srb_, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                context_->SetVertexBuffers(0, 0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
                context_->SetIndexBuffer(nullptr, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

                Diligent::DrawAttribs draw_attributes;
                draw_attributes.NumVertices = 3;
                draw_attributes.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
                context_->Draw(draw_attributes);
            }
        }
        
        void GraphicsManager::render_()
        {
            Diligent::ITextureView* pRTV = g_buffer_.color_texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
            Diligent::ITextureView* pDSV = g_buffer_.depth_texture->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
            
            context_->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            
            {
                // Clear the back buffer, transitions is not needed
                const float clear_color[4] = {};
                context_->ClearRenderTarget(pRTV, clear_color, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
                context_->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

                {
                    Diligent::GlobalConstants constants;
                    constants.camera_view_projection = camera_view_projection_.Transpose();
                    constants.camera_view_projection_inverse = camera_view_projection_.Inverse().Transpose();
                    constants.camera_position = camera_position_;
                    constants.sun_direction = normalize(-sun_direction_);

                    context_->UpdateBuffer(global_constants_, 0, sizeof(constants), &constants, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }
                
                render_plane_();
                render_sphere_();
            }
            
            context_->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
            
            render_post_process_();
        }

        void GraphicsManager::present_()
        {
            assert(swap_chain_);
            assert(context_);
            
            context_->Flush();
            context_->FinishFrame();
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

        void GraphicsManager::create_sphere_pso_()
        {
            auto *engine_factory = Diligent::GetEngineFactoryMtl();

            // Create a shader source stream factory to load shaders from files.
            Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> shader_source_factory;
            engine_factory->CreateDefaultShaderSourceStreamFactory(nullptr, &shader_source_factory);

            SHADER_INFO vertex_shader;
            vertex_shader.name = "Sphere vertex shader";
            vertex_shader.path = assets_path_ + "/texture.vsh";

            SHADER_INFO pixel_shader;
            pixel_shader.name = "Sphere pixel shader";
            pixel_shader.path = assets_path_ + "/texture.psh";

            PSO_INFO pso_info;
            pso_info.name = "Sphere PSO";
            pso_info.rtv_format = swap_chain_->GetDesc().ColorBufferFormat;
            pso_info.dsv_format = swap_chain_->GetDesc().DepthBufferFormat;
            pso_info.shader_source_factory = shader_source_factory;
            pso_info.vertex_shader = vertex_shader;
            pso_info.pixel_shader = pixel_shader;
            pso_info.components = VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD;
            pso_info.cull_mode = Diligent::CULL_MODE_BACK;
            pso_info.depth_enable = true;
            pso_info.depth_write_enable = true;

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

            sphere_pso_ = create_pipeline_state(device_, pso_info);

            sphere_pso_->CreateShaderResourceBinding(&sphere_srb_, true);
        }

        void GraphicsManager::create_plane_pso_()
        {
            auto *engine_factory = Diligent::GetEngineFactoryMtl();

            // Create a shader source stream factory to load shaders from files.
            Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> shader_source_factory;
            engine_factory->CreateDefaultShaderSourceStreamFactory(nullptr, &shader_source_factory);

            SHADER_INFO vertex_shader;
            vertex_shader.name = "Plane vertex shader";
            vertex_shader.path = assets_path_ + "/texture.vsh";

            SHADER_INFO pixel_shader;
            pixel_shader.name = "Plane pixel shader";
            pixel_shader.path = assets_path_ + "/texture.psh";

            PSO_INFO pso_info;
            pso_info.name = "Plane PSO";
            pso_info.rtv_format = swap_chain_->GetDesc().ColorBufferFormat;
            pso_info.dsv_format = swap_chain_->GetDesc().DepthBufferFormat;
            pso_info.shader_source_factory = shader_source_factory;
            pso_info.vertex_shader = vertex_shader;
            pso_info.pixel_shader = pixel_shader;
            pso_info.components = VERTEX_COMPONENT_FLAG_POSITION_NORMAL_TEXCOORD;
            pso_info.cull_mode = Diligent::CULL_MODE_BACK;
            pso_info.depth_enable = true;
            pso_info.depth_write_enable = true;
        
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

                Diligent::TEXTURE_ADDRESS_MIRROR, 
                Diligent::TEXTURE_ADDRESS_MIRROR, 
                Diligent::TEXTURE_ADDRESS_MIRROR
            };
            Diligent::ImmutableSamplerDesc immutable_samplers[] = 
            {
                {Diligent::SHADER_TYPE_PIXEL, "g_Texture", sampler_linear_clamp_desc}
            };
            pso_info.immutable_samplers = immutable_samplers;
            pso_info.nb_immutable_samplers = _countof(immutable_samplers);

            plane_pso_ = create_pipeline_state(device_, pso_info);

            plane_pso_->CreateShaderResourceBinding(&plane_srb_, true);
        }

        void GraphicsManager::create_post_process_pso_()
        {
            assert(swap_chain_);
            assert(device_);
            
            auto *engine_factory = Diligent::GetEngineFactoryMtl();

            // Create a shader source stream factory to load shaders from files.
            Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> shader_source_factory;
            engine_factory->CreateDefaultShaderSourceStreamFactory(nullptr, &shader_source_factory);

            SHADER_INFO vertex_shader;
            vertex_shader.name = "Post process vertex shader";
            vertex_shader.path = assets_path_ + "/post_process.vsh";

            SHADER_INFO pixel_shader;
            pixel_shader.name = "Post process pixel shader";
            pixel_shader.path = assets_path_ + "/post_process.psh";

            PSO_INFO pso_info;
            pso_info.name = "Post process PSO";
            pso_info.rtv_format = swap_chain_->GetDesc().ColorBufferFormat;
            pso_info.shader_source_factory = shader_source_factory;
            pso_info.vertex_shader = vertex_shader;
            pso_info.pixel_shader = pixel_shader;
            pso_info.depth_enable = false;
            pso_info.depth_write_enable = false;
            pso_info.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

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
                {Diligent::SHADER_TYPE_PIXEL, "g_GBuffer_Color_sampler", sampler_linear_clamp_desc},
            };
            pso_info.immutable_samplers = immutable_samplers;
            pso_info.nb_immutable_samplers = _countof(immutable_samplers);

            post_process_pso_ = create_pipeline_state(device_, pso_info);
        }
    }
}
