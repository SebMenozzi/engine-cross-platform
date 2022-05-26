#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <fstream>
#include <iostream>

#include <Errors.hpp>
#if METAL_SUPPORTED
#include "EngineFactoryMtl.h"
#endif
#include <EngineFactory.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>
#include <Timer.hpp>
#include <MapHelper.hpp>
#include <NativeWindow.h>
#include <GraphicsUtilities.h> // CreateUniformBuffer
#include <BasicMath.hpp>

#include "utils_maths.hpp"

#include "object.hpp"
#include "cube.hpp"
#include "plane.hpp"
#include "chunk.hpp"
#include "sphere/uv_sphere.hpp"

using Buffer = std::vector<uint8_t>;

namespace engine
{
    namespace graphics
    {
        class GraphicsManager
        {
            public:
                GraphicsManager();

                void initialize(const Diligent::NativeWindow* window);
                void update(double dt);
                void shutdown();
                void resize(uint32_t width, uint32_t height);
                void set_fov(double fov);
                void set_camera_view(Diligent::float4x4 camera_view);

            private:
                void update_(double dt);
                void render_();
                void present_();

                Diligent::float4x4 get_adjusted_projection_matrix_(float fov, float near, float far) const;
                bool create_device_and_swap_chain_metal_(const Diligent::NativeWindow* window);
                void create_swap_chain_metal_(const Diligent::NativeWindow* window);

                void create_chunk_pso_();
                void create_cube_pso_();
                void create_sphere_pso_();
                void create_plane_pso_();

                void render_chunk_();
                void render_cube_();
                void render_sphere_();
                void render_plane_();

                /// MARK: - Rendering
                Diligent::IRenderDevice* device_ = nullptr;
                Diligent::IDeviceContext* context_ = nullptr;
                Diligent::ISwapChain* swap_chain_ = nullptr;
                Diligent::SwapChainDesc swap_chain_desc_;

                bool vsync_enabled_ = false;

                /// MARK: - Chunk
                Diligent::RefCntAutoPtr<Diligent::IPipelineState> chunk_pso_;
                Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> chunk_srb_;

                Diligent::RefCntAutoPtr<Diligent::IBuffer> chunk_vertex_buffer_;

                /// MARK: - Cube
                Diligent::RefCntAutoPtr<Diligent::IPipelineState> cube_pso_;
                Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> cube_srb_;

                Diligent::RefCntAutoPtr<Diligent::ITextureView> cube_texture_srv_;
                Diligent::RefCntAutoPtr<Diligent::IBuffer> cube_vertex_buffer_;
                Diligent::RefCntAutoPtr<Diligent::IBuffer> cube_index_buffer_;

                /// MARK: - Sphere
                Diligent::RefCntAutoPtr<Diligent::IPipelineState> sphere_pso_;
                Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> sphere_srb_;

                Diligent::RefCntAutoPtr<Diligent::ITextureView> sphere_texture_srv_;
                Diligent::RefCntAutoPtr<Diligent::IBuffer> sphere_vertex_buffer_;
                Diligent::RefCntAutoPtr<Diligent::IBuffer> sphere_index_buffer_;

                /// MARK: - Plane
                Diligent::RefCntAutoPtr<Diligent::IPipelineState> plane_pso_;
                Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> plane_srb_;

                Diligent::RefCntAutoPtr<Diligent::IBuffer> plane_vertex_buffer_;
                Diligent::RefCntAutoPtr<Diligent::IBuffer> plane_index_buffer_;

                Diligent::RefCntAutoPtr<Diligent::IBuffer> vertices_constants_;
                Diligent::float4x4 world_view_projection_matrix_;

                /// MARK: - Camera
                Diligent::float4x4 camera_view_;
                //std::shared_ptr<camera::Camera> camera_;
                double fov_ = 45.0;
        };
    }
}
