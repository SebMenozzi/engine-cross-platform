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

            std::vector<Diligent::StateTransitionDesc> barriers;
            
            // Create buffer for constants that is shared between all PSOs
            {
                Diligent::BufferDesc buffer_desc;
                buffer_desc.Name = "Global constants buffer";
                buffer_desc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
                buffer_desc.Size = sizeof(Diligent::GlobalConstants);
                device_->CreateBuffer(buffer_desc, nullptr, &global_constants_);
            }
            //Diligent::CreateUniformBuffer(device_, sizeof(Diligent::GlobalConstants), "Global constants", &global_constants_);
            
            barriers.emplace_back(global_constants_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
            
            //barriers.emplace_back(g_buffer_.color_texture, Diligent::RESOURCE_STATE_RENDER_TARGET, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

            // MARK: - Sphere

            create_sphere_pso_();

            auto sphere = object::sphere::UVSphere(1.0, 100.0, 100.0);

            std::vector<Diligent::Vertex> sphere_vertices = create_vertices(
                sphere.positions_,
                sphere.normals_,
                sphere.textcoords_
            );

            sphere_vertex_buffer_ = create_vertex_buffer(device_, sphere_vertices);
            barriers.emplace_back(sphere_vertex_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

            sphere_indice_buffer_ = create_indice_buffer(device_, sphere.indices_);
            barriers.emplace_back(sphere_indice_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

            // MARK: - Plane

            create_plane_pso_();

            std::vector<Diligent::Vertex> plane_vertices = create_vertices(
                object::PLANE_POSITIONS,
                object::PLANE_NORMALS,
                object::PLANE_TEXTCOORDS
            );

            plane_vertex_buffer_ = create_vertex_buffer(device_, plane_vertices);
            barriers.emplace_back(plane_vertex_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

            plane_indice_buffer_ = create_indice_buffer(device_, object::PLANE_INDICES);
            barriers.emplace_back(plane_indice_buffer_, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

            // MARK: - Texture

            auto wood_texture = load_texture(device_, assets_path_ + "/wood.jpeg");
            auto mj_texture = load_texture(device_, assets_path_ + "/mj.jpg");

            sphere_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mj_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
            plane_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(wood_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));

            barriers.emplace_back(mj_texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
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
                //post_process_srb_->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_GBuffer_Depth")->Set(g_buffer_.depth_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
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
            // Bind vertex and indice buffers
            Diligent::IBuffer* buffers[] = { sphere_vertex_buffer_ };
            context_->SetVertexBuffers(0, 1, buffers, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
            context_->SetIndexBuffer(sphere_indice_buffer_, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            /// Set the pipeline state in the immediate context
            context_->SetPipelineState(sphere_pso_);

            // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
            // makes sure that resources are transitioned to required states.
            context_->CommitShaderResources(sphere_srb_, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            
            const Diligent::Uint32 nb_elements = Diligent::Uint32(sphere_indice_buffer_->GetDesc().Size);
            const Diligent::Uint32 element_size = sizeof(Diligent::Uint32);

            Diligent::DrawIndexedAttribs draw_attributes(nb_elements / element_size, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL);
            context_->DrawIndexed(draw_attributes);
        }

        void GraphicsManager::render_plane_()
        {
            // Bind vertex and indice buffers
            Diligent::IBuffer* buffers[] = { plane_vertex_buffer_ };
            context_->SetVertexBuffers(0, 1, buffers, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
            context_->SetIndexBuffer(plane_indice_buffer_, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            /// Set the pipeline state in the immediate context
            context_->SetPipelineState(plane_pso_);

            // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
            // makes sure that resources are transitioned to required states.
            context_->CommitShaderResources(plane_srb_, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            
            const Diligent::Uint32 nb_elements = Diligent::Uint32(plane_indice_buffer_->GetDesc().Size);
            const Diligent::Uint32 element_size = sizeof(Diligent::Uint32);

            Diligent::DrawIndexedAttribs draw_attributes(nb_elements / element_size, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL);
            context_->DrawIndexed(draw_attributes);
        }

        void GraphicsManager::render_post_process_()
        {
            Diligent::ITextureView* pRTV = swap_chain_->GetCurrentBackBufferRTV();
            Diligent::ITextureView* pDSV = swap_chain_->GetDepthBufferDSV();
            
            context_->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            const float clear_color[] = {0.01f, 0.01f, 0.01f, 1.0f};
            context_->ClearRenderTarget(pRTV, clear_color, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            context_->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            
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
            
            {
                context_->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                
                const float clear_color[4] = {};
                context_->ClearRenderTarget(pRTV, clear_color, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
                context_->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

                {
                    const auto& swap_chain_desc = swap_chain_->GetDesc();
                    
                    Diligent::GlobalConstants constants;
                    constants.camera_view_projection = camera_view_projection_.Transpose();
                    constants.viewport_size = Diligent::float4(
                        static_cast<float>(swap_chain_desc.Width),
                        static_cast<float>(swap_chain_desc.Height),
                        1.f / static_cast<float>(swap_chain_desc.Width),
                        1.f / static_cast<float>(swap_chain_desc.Height)
                    );
                    
                    context_->UpdateBuffer(global_constants_, 0, static_cast<Diligent::Uint32>(sizeof(constants)), &constants, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }
                
                render_plane_();
                render_sphere_();
                
                context_->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
            }
            
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

            sphere_pso_ = create_pipeline_state(device_, pso_info, 0, 0);

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

            plane_pso_ = create_pipeline_state(device_, pso_info, 0, 0);

            plane_pso_->CreateShaderResourceBinding(&plane_srb_, true);
            
            plane_srb_->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Constants")->Set(global_constants_);
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
            vertex_shader.path = assets_path_ + "/invert.vsh";

            SHADER_INFO pixel_shader;
            pixel_shader.name = "Post process pixel shader";
            pixel_shader.path = assets_path_ + "/invert.psh";

            PSO_INFO pso_info;
            pso_info.name = "Post process PSO";
            pso_info.rtv_format = swap_chain_->GetDesc().ColorBufferFormat;
            pso_info.dsv_format = swap_chain_->GetDesc().DepthBufferFormat;
            pso_info.shader_source_factory = shader_source_factory;
            pso_info.vertex_shader = vertex_shader;
            pso_info.pixel_shader = pixel_shader;
            pso_info.depth_enable = false;
            pso_info.depth_write_enable = false;
            pso_info.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            
            Diligent::ShaderResourceVariableDesc variables[] =
            {
                {Diligent::SHADER_TYPE_PIXEL, "g_GBuffer_Color", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
                {Diligent::SHADER_TYPE_PIXEL, "g_GBuffer_Depth", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
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
                {Diligent::SHADER_TYPE_PIXEL, "g_GBuffer_Color_sampler", sampler_linear_clamp_desc},
            };
            pso_info.immutable_samplers = immutable_samplers;
            pso_info.nb_immutable_samplers = _countof(immutable_samplers);

            post_process_pso_ = create_pipeline_state(device_, pso_info, 0, 0);
        }

        void GraphicsManager::create_scene_materials_(
            Diligent::uint2& cube_material_range, 
            Diligent::Uint32& ground_material, 
            std::vector<Diligent::Material>& materials
        )
        {
            Diligent::Uint32 anisotropic_clamp_sampler_index = 0;
            Diligent::Uint32 anisotropic_wrap_sampler_index = 0;

            // Create samplers
            {
                const Diligent::SamplerDesc anisotropic_clamp_sampler {
                    Diligent::FILTER_TYPE_ANISOTROPIC, 
                    Diligent::FILTER_TYPE_ANISOTROPIC, 
                    Diligent::FILTER_TYPE_ANISOTROPIC,
                    Diligent::TEXTURE_ADDRESS_CLAMP, 
                    Diligent::TEXTURE_ADDRESS_CLAMP, 
                    Diligent::TEXTURE_ADDRESS_CLAMP, 
                    0.f, 8
                };

                const Diligent::SamplerDesc anisotropic_wrap_sampler {
                    Diligent::FILTER_TYPE_ANISOTROPIC, 
                    Diligent::FILTER_TYPE_ANISOTROPIC, 
                    Diligent::FILTER_TYPE_ANISOTROPIC,
                    Diligent::TEXTURE_ADDRESS_WRAP, 
                    Diligent::TEXTURE_ADDRESS_WRAP, 
                    Diligent::TEXTURE_ADDRESS_WRAP, 
                    0.f, 8
                };

                Diligent::RefCntAutoPtr<Diligent::ISampler> sampler;
                device_->CreateSampler(anisotropic_clamp_sampler, &sampler);
                anisotropic_clamp_sampler_index = static_cast<Diligent::Uint32>(scene_.samplers.size());
                scene_.samplers.push_back(std::move(sampler));

                sampler = nullptr;
                device_->CreateSampler(anisotropic_wrap_sampler, &sampler);
                anisotropic_wrap_sampler_index = static_cast<Diligent::Uint32>(scene_.samplers.size());
                scene_.samplers.push_back(std::move(sampler));
            }

            const auto load_material = [&](
                const char* color_map_name, 
                const Diligent::float4& base_color,
                Diligent::Uint32 sampler_index
            )
            {
                Diligent::TextureLoadInfo load_info;
                load_info.IsSRGB = true;
                load_info.GenerateMips = true;
                Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
                Diligent::CreateTextureFromFile(color_map_name, load_info, device_, &texture);
                VERIFY_EXPR(texture);

                Diligent::Material material;
                material.sampler_index = sampler_index;
                material.base_color_mask = base_color;
                material.base_color_texture_index = static_cast<Diligent::Uint32>(scene_.textures.size());
                scene_.textures.push_back(std::move(texture));
                materials.push_back(material);
            };

            // Cube materials
            cube_material_range.x = static_cast<Diligent::Uint32>(materials.size());
            load_material("DGLogo0.png", Diligent::float4{1.f}, anisotropic_clamp_sampler_index);
            load_material("DGLogo1.png", Diligent::float4{1.f}, anisotropic_clamp_sampler_index);
            load_material("DGLogo2.png", Diligent::float4{1.f}, anisotropic_clamp_sampler_index);
            load_material("DGLogo3.png", Diligent::float4{1.f}, anisotropic_clamp_sampler_index);
            cube_material_range.y = static_cast<Diligent::Uint32>(materials.size());

            // Ground material
            ground_material = static_cast<Diligent::Uint32>(materials.size());
            load_material("Marble.jpg", Diligent::float4{1.f}, anisotropic_wrap_sampler_index);
        }

        Mesh GraphicsManager::create_plane_mesh_()
        {
            Mesh mesh;
            mesh.name = "Plane";

            std::vector<Diligent::Vertex> vertices = create_vertices(
                object::PLANE_POSITIONS,
                object::PLANE_NORMALS,
                object::PLANE_TEXTCOORDS
            );

            mesh.nb_vertices = static_cast<Diligent::Uint32>(vertices.size());

            Diligent::RefCntAutoPtr<Diligent::IBuffer> vertex_buffer = create_vertex_buffer(
                device_, 
                vertices,
                Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RAY_TRACING,
                Diligent::BUFFER_MODE_STRUCTURED
            );

            mesh.vertex_buffer = vertex_buffer;

            Diligent::RefCntAutoPtr<Diligent::IBuffer> indice_buffer = create_indice_buffer(
                device_,
                object::PLANE_INDICES,
                Diligent::BIND_INDEX_BUFFER | Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RAY_TRACING,
                Diligent::BUFFER_MODE_STRUCTURED
            );

            mesh.indice_buffer = indice_buffer;

            return mesh;
        }

        Mesh GraphicsManager::create_cube_mesh_()
        {
            Mesh mesh;
            mesh.name = "Cube";

            std::vector<Diligent::Vertex> vertices = create_vertices(
                object::CUBE_POSITIONS,
                object::CUBE_NORMALS,
                object::CUBE_TEXTCOORDS
            );

            mesh.nb_vertices = static_cast<Diligent::Uint32>(vertices.size());

            Diligent::RefCntAutoPtr<Diligent::IBuffer> vertex_buffer = create_vertex_buffer(
                device_, 
                vertices,
                Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RAY_TRACING,
                Diligent::BUFFER_MODE_STRUCTURED
            );

            mesh.vertex_buffer = vertex_buffer;

            Diligent::RefCntAutoPtr<Diligent::IBuffer> indice_buffer = create_indice_buffer(
                device_,
                object::CUBE_INDICES,
                Diligent::BIND_INDEX_BUFFER | Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RAY_TRACING,
                Diligent::BUFFER_MODE_STRUCTURED
            );

            mesh.indice_buffer = indice_buffer;

            return mesh;
        }

        void GraphicsManager::create_scene_objects_(
            Diligent::uint2& cube_material_range, 
            Diligent::Uint32& ground_material
        )
        {
            Diligent::Uint32 cube_mesh_id = 0;
            Diligent::Uint32 plane_mesh_id = 0;

            auto cube_mesh = create_plane_mesh_();
            auto plane_mesh = create_plane_mesh_();

            const auto rt_props = device_->GetAdapterInfo().RayTracing;

            // Cube mesh will be copied to the beginning of the buffers
            cube_mesh.first_vertex = 0;
            cube_mesh.first_indice = 0;

            // Plane mesh data will reside after the cube. Offsets must be properly aligned!
            plane_mesh.first_vertex = Diligent::AlignUp(cube_mesh.nb_vertices * Diligent::Uint32{sizeof(Diligent::Vertex)}, rt_props.VertexBufferAlignment) / sizeof(Diligent::Vertex);
            plane_mesh.first_indice = Diligent::AlignUp(cube_mesh.nb_indices * Diligent::Uint32{sizeof(uint)}, rt_props.IndexBufferAlignment) / sizeof(uint);

            // Merge vertex buffers
            {
                Diligent::BufferDesc vertex_buffer_desc;
                vertex_buffer_desc.Name = "Shared vertex buffer";
                vertex_buffer_desc.BindFlags = Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RAY_TRACING;
                vertex_buffer_desc.Size = (Diligent::Uint64{plane_mesh.first_vertex} + Diligent::Uint64{plane_mesh.nb_vertices}) * sizeof(Diligent::Vertex);
                vertex_buffer_desc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
                vertex_buffer_desc.ElementByteStride = sizeof(Diligent::Vertex);

                Diligent::RefCntAutoPtr<Diligent::IBuffer> shared_vertex_buffer;
                device_->CreateBuffer(vertex_buffer_desc, nullptr, &shared_vertex_buffer);

                // Copy cube vertices
                context_->CopyBuffer(
                    cube_mesh.vertex_buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                    shared_vertex_buffer, cube_mesh.first_vertex * sizeof(Diligent::Vertex), cube_mesh.nb_vertices * sizeof(Diligent::Vertex),
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
                );

                // Copy plane vertices
                context_->CopyBuffer(
                    plane_mesh.vertex_buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                    shared_vertex_buffer, plane_mesh.first_vertex * sizeof(Diligent::Vertex), plane_mesh.nb_vertices * sizeof(Diligent::Vertex),
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
                );

                cube_mesh.vertex_buffer = shared_vertex_buffer;
                plane_mesh.vertex_buffer = shared_vertex_buffer;
            }

            // Merge indice buffers
            {
                Diligent::BufferDesc indice_buffer_desc;
                indice_buffer_desc.Name = "Shared indice buffer";
                indice_buffer_desc.BindFlags = Diligent::BIND_INDEX_BUFFER | Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RAY_TRACING;
                indice_buffer_desc.Size = (Diligent::Uint64{plane_mesh.first_indice} + Diligent::Uint64{plane_mesh.nb_vertices}) * sizeof(uint);
                indice_buffer_desc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
                indice_buffer_desc.ElementByteStride = sizeof(uint);

                Diligent::RefCntAutoPtr<Diligent::IBuffer> shared_indice_buffer;
                device_->CreateBuffer(indice_buffer_desc, nullptr, &shared_indice_buffer);

                // Copy cube indices
                context_->CopyBuffer(
                    cube_mesh.indice_buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                    shared_indice_buffer, cube_mesh.first_indice * sizeof(uint), cube_mesh.nb_indices * sizeof(uint),
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
                );

                // Copy plane indices
                context_->CopyBuffer(
                    plane_mesh.indice_buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                    shared_indice_buffer, plane_mesh.first_indice * sizeof(uint), plane_mesh.nb_indices * sizeof(uint),
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
                );

                cube_mesh.indice_buffer = shared_indice_buffer;
                plane_mesh.indice_buffer = shared_indice_buffer;
            }

            cube_mesh_id = static_cast<Diligent::Uint32>(scene_.meshes.size());
            scene_.meshes.push_back(cube_mesh);
            plane_mesh_id = static_cast<Diligent::Uint32>(scene_.meshes.size());
            scene_.meshes.push_back(plane_mesh);

            // Create cube objects
            const auto add_cube_object = [&](float angle, float x, float y, float z, float scale, bool is_dynamic = false)
            {
                const auto model_matrix = Diligent::float4x4::RotationY(angle * Diligent::PI_F) * Diligent::float4x4::Scale(scale) * Diligent::float4x4::Translation(x * 2.0f, y * 2.0f - 1.0f, z * 2.0f);

                Diligent::Object object;
                object.model_matrix = model_matrix.Transpose();
                object.normal_matrix = object.model_matrix;
                object.material_id  = (scene_.objects.size() % (cube_material_range.y - cube_material_range.x)) + cube_material_range.x;
                object.mesh_id = cube_mesh_id;
                object.first_indice = scene_.meshes[object.mesh_id].first_indice;
                object.first_vertex = scene_.meshes[object.mesh_id].first_vertex;
                scene_.objects.push_back(object);

                if (is_dynamic)
                {
                    DynamicObject dynamic_object;
                    dynamic_object.object_index = static_cast<Diligent::Uint32>(scene_.objects.size() - 1);
                    scene_.dynamic_objects.push_back(dynamic_object);
                }
            };

            add_cube_object(0.25f,  0.0f, 1.00f,  1.5f, 0.9f);
            add_cube_object(0.00f, -1.9f, 1.00f, -0.5f, 0.5f);
            add_cube_object(0.00f, -1.0f, 1.00f,  0.0f, 1.0f);
            add_cube_object(0.30f, -0.2f, 1.00f, -1.0f, 0.7f);
            add_cube_object(0.25f, -1.7f, 1.00f, -1.6f, 1.1f, true);
            add_cube_object(0.28f,  0.7f, 1.00f,  3.0f, 1.3f);
            add_cube_object(0.10f,  1.5f, 1.00f,  1.0f, 1.1f);
            add_cube_object(0.21f, -3.2f, 1.00f,  0.2f, 1.2f);
            add_cube_object(0.05f, -2.1f, 1.00f,  1.6f, 1.1f);
            
            add_cube_object(0.04f, -1.4f, 2.18f, -1.4f, 0.6f);
            add_cube_object(0.24f, -1.0f, 2.10f,  0.5f, 1.1f, true);
            add_cube_object(0.02f, -0.5f, 2.00f, -0.9f, 0.9f);
            add_cube_object(0.08f, -1.7f, 1.96f,  1.7f, 0.7f);
            add_cube_object(0.17f,  1.5f, 2.00f,  1.1f, 0.9f);
            
            add_cube_object(0.6f, -1.0f, 3.25f, -0.2f, 1.2f);

            InstancedObjects instanced_objects;
            instanced_objects.mesh_index = cube_mesh_id;
            instanced_objects.nb_objects = static_cast<Diligent::Uint32>(scene_.objects.size());
            instanced_objects.object_offset = 0;

            scene_.instanced_objects.push_back(instanced_objects);

            // Create ground plane object
            instanced_objects.object_offset = static_cast<Diligent::Uint32>(scene_.objects.size());
            instanced_objects.mesh_index = plane_mesh_id;

            {
                Diligent::Object object;
                object.model_matrix = (Diligent::float4x4::Scale(50.f, 1.f, 50.f) * Diligent::float4x4::Translation(0.f, -0.2f, 0.f)).Transpose();
                object.normal_matrix = Diligent::float3x3::Identity();
                object.material_id  = ground_material;
                object.mesh_id = plane_mesh_id;
                object.first_indice = scene_.meshes[object.mesh_id].first_indice;
                object.first_vertex = scene_.meshes[object.mesh_id].first_vertex;

                scene_.objects.push_back(object);
            }

            instanced_objects.nb_objects = static_cast<Diligent::Uint32>(scene_.objects.size()) - instanced_objects.object_offset;
            scene_.instanced_objects.push_back(instanced_objects);
        }

        void GraphicsManager::create_scene_acceleration_structs_()
        {
            // Create and build bottom-level acceleration structure
            {
                Diligent::RefCntAutoPtr<Diligent::IBuffer> scratch_buffer;

                for (auto& mesh : scene_.meshes)
                {
                    // Create BLAS
                    Diligent::BLASTriangleDesc triangles;
                    {
                        triangles.GeometryName = mesh.name.c_str();
                        triangles.MaxVertexCount = mesh.nb_vertices;
                        triangles.VertexValueType = Diligent::VT_FLOAT32;
                        triangles.VertexComponentCount = 3;
                        triangles.MaxPrimitiveCount = mesh.nb_indices / 3;
                        triangles.IndexType = Diligent::VT_UINT32;

                        const auto blas_name{mesh.name + " BLAS"};

                        Diligent::BottomLevelASDesc as_desc;
                        as_desc.Name = blas_name.c_str();
                        as_desc.Flags = Diligent::RAYTRACING_BUILD_AS_PREFER_FAST_TRACE;
                        as_desc.pTriangles = &triangles;
                        as_desc.TriangleCount = 1;

                        device_->CreateBLAS(as_desc, &mesh.blas);
                    }

                    // Create or reuse scratch buffer; this will insert the barrier between BuildBLAS invocations, which may be suboptimal.
                    if (!scratch_buffer || scratch_buffer->GetDesc().Size < mesh.blas->GetScratchBufferSizes().Build)
                    {
                        Diligent::BufferDesc buffer_desc;
                        buffer_desc.Name = "BLAS Scratch Buffer";
                        buffer_desc.Usage = Diligent::USAGE_DEFAULT;
                        buffer_desc.BindFlags = Diligent::BIND_RAY_TRACING;
                        buffer_desc.Size = mesh.blas->GetScratchBufferSizes().Build;

                        scratch_buffer = nullptr;
                        device_->CreateBuffer(buffer_desc, nullptr, &scratch_buffer);
                    }

                    // Build BLAS
                    Diligent::BLASBuildTriangleData triangle_data;
                    triangle_data.GeometryName = triangles.GeometryName;
                    triangle_data.pVertexBuffer = mesh.vertex_buffer;
                    triangle_data.VertexStride = mesh.vertex_buffer->GetDesc().ElementByteStride;
                    triangle_data.VertexOffset = Diligent::Uint64{mesh.first_vertex} * Diligent::Uint64{triangle_data.VertexStride};
                    triangle_data.VertexCount = mesh.nb_vertices;
                    triangle_data.VertexValueType = triangles.VertexValueType;
                    triangle_data.VertexComponentCount = triangles.VertexComponentCount;
                    triangle_data.pIndexBuffer = mesh.indice_buffer;
                    triangle_data.IndexOffset = Diligent::Uint64{mesh.first_indice} * Diligent::Uint64{mesh.indice_buffer->GetDesc().ElementByteStride};
                    triangle_data.PrimitiveCount = triangles.MaxPrimitiveCount;
                    triangle_data.IndexType = triangles.IndexType;
                    triangle_data.Flags = Diligent::RAYTRACING_GEOMETRY_FLAG_OPAQUE;

                    Diligent::BuildBLASAttribs attributes;
                    attributes.pBLAS = mesh.blas;
                    attributes.pTriangleData = &triangle_data;
                    attributes.TriangleDataCount = 1;

                    // Scratch buffer will be used to store temporary data during the BLAS build.
                    // Previous content in the scratch buffer will be discarded.
                    attributes.pScratchBuffer = scratch_buffer;

                    // Allow engine to change resource states.
                    attributes.BLASTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
                    attributes.GeometryTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
                    attributes.ScratchBufferTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

                    context_->BuildBLAS(attributes);
                }
            }

            // Create TLAS
            {
                Diligent::TopLevelASDesc tlas_desc;
                tlas_desc.Name = "Scene TLAS";
                tlas_desc.MaxInstanceCount = static_cast<Diligent::Uint32>(scene_.objects.size());
                tlas_desc.Flags = Diligent::RAYTRACING_BUILD_AS_ALLOW_UPDATE | Diligent::RAYTRACING_BUILD_AS_PREFER_FAST_TRACE;
                device_->CreateTLAS(tlas_desc, &scene_.tlas);
            }
        }

        void GraphicsManager::update_tlas_()
        {
            const Diligent::Uint32 nb_instances = static_cast<Diligent::Uint32>(scene_.objects.size());
            bool update = true;

            // Create scratch buffer
            if (!scene_.tlas_scratch_buffer)
            {
                Diligent::BufferDesc buffer_desc;
                buffer_desc.Name = "TLAS Scratch Buffer";
                buffer_desc.Usage = Diligent::USAGE_DEFAULT;
                buffer_desc.BindFlags = Diligent::BIND_RAY_TRACING;
                buffer_desc.Size = std::max(scene_.tlas->GetScratchBufferSizes().Build, scene_.tlas->GetScratchBufferSizes().Update);
                device_->CreateBuffer(buffer_desc, nullptr, &scene_.tlas_scratch_buffer);
                update = false; // this is the first build
            }

            // Create instance buffer
            if (!scene_.tlas_instances_buffer)
            {
                Diligent::BufferDesc buffer_desc;
                buffer_desc.Name = "TLAS Instance Buffer";
                buffer_desc.Usage = Diligent::USAGE_DEFAULT;
                buffer_desc.BindFlags = Diligent::BIND_RAY_TRACING;
                buffer_desc.Size = Diligent::Uint64{Diligent::TLAS_INSTANCE_DATA_SIZE} * Diligent::Uint64{nb_instances};
                device_->CreateBuffer(buffer_desc, nullptr, &scene_.tlas_instances_buffer);
            }

            // Setup instances
            std::vector<Diligent::TLASBuildInstanceData> instances(nb_instances);
            std::vector<std::string> instance_names(nb_instances);

            for (Diligent::Uint32 i = 0; i < nb_instances; ++i)
            {
                const auto& object = scene_.objects[i];
                Diligent::TLASBuildInstanceData& instance = instances[i];
                std::string& name = instance_names[i];
                const auto& mesh = scene_.meshes[object.mesh_id];
                const auto model_matrix = object.model_matrix.Transpose();

                name = mesh.name + " Instance (" + std::to_string(i) + ")";

                instance.InstanceName = name.c_str();
                instance.pBLAS = mesh.blas.RawPtr<Diligent::IBottomLevelAS>();
                instance.Mask = 0xFF;

                // CustomId will be read in shader by RayQuery::CommittedInstanceID()
                instance.CustomId = i;

                instance.Transform.SetRotation(model_matrix.Data(), 4);
                instance.Transform.SetTranslation(model_matrix.m30, model_matrix.m31, model_matrix.m32);
            }

            // Build  TLAS
            Diligent::BuildTLASAttribs attributes;
            attributes.pTLAS = scene_.tlas;
            attributes.Update = update;

            // Scratch buffer will be used to store temporary data during TLAS build or update.
            // Previous content in the scratch buffer will be discarded.
            attributes.pScratchBuffer = scene_.tlas_scratch_buffer;

            // Instance buffer will store instance data during TLAS build or update.
            // Previous content in the instance buffer will be discarded.
            attributes.pInstanceBuffer = scene_.tlas_instances_buffer;

            // Instances will be converted to the format that is required by the graphics driver and copied to the instance buffer.
            attributes.pInstances = instances.data();
            attributes.InstanceCount = nb_instances;

            // Allow engine to change resource states.
            attributes.TLASTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            attributes.BLASTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            attributes.InstanceBufferTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            attributes.ScratchBufferTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

            context_->BuildTLAS(attributes);
        }

        void GraphicsManager::create_scene_()
        {
            Diligent::uint2 cube_material_range;
            Diligent::Uint32 ground_material;
            std::vector<Diligent::Material> materials;

            create_scene_materials_(cube_material_range, ground_material, materials);
            create_scene_objects_(cube_material_range, ground_material);
            create_scene_acceleration_structs_();

            // Create buffer for object attribs
            {
                Diligent::BufferDesc buffer_desc;
                buffer_desc.Name = "Object attribs buffer";
                buffer_desc.Usage = Diligent::USAGE_DEFAULT;
                buffer_desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
                buffer_desc.Size = static_cast<Diligent::Uint64>(sizeof(scene_.objects[0]) * scene_.objects.size());
                buffer_desc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
                buffer_desc.ElementByteStride = sizeof(scene_.objects[0]);

                device_->CreateBuffer(buffer_desc, nullptr, &scene_.objects_buffer);
            }

            // Create and initialize buffer for material attribs
            {
                Diligent::BufferDesc buffer_desc;
                buffer_desc.Name = "Material attribs buffer";
                buffer_desc.Usage = Diligent::USAGE_DEFAULT;
                buffer_desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
                buffer_desc.Size = static_cast<Diligent::Uint64>(sizeof(materials[0]) * materials.size());
                buffer_desc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
                buffer_desc.ElementByteStride = sizeof(materials[0]);

                Diligent::BufferData buffer_data{materials.data(), buffer_desc.Size};

                device_->CreateBuffer(buffer_desc, &buffer_data, &scene_.materials_buffer);
            }

            // Create dynamic buffer for scene object constants (unique for each draw call)
            {
                Diligent::BufferDesc buffer_desc;
                buffer_desc.Name = "Global constants buffer";
                buffer_desc.Usage = Diligent::USAGE_DYNAMIC;
                buffer_desc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
                buffer_desc.Size = sizeof(Diligent::ObjectConstants);
                buffer_desc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

                device_->CreateBuffer(buffer_desc, nullptr, &scene_.object_constants);
            }
        }
    }
}
