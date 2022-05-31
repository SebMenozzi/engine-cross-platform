#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <fstream>
#include <iostream>
#include <cstring>

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

#include "graphics_utils.hpp"
#include "cube.hpp"
#include "plane.hpp"
#include "sphere/uv_sphere.hpp"

using Buffer = std::vector<uint8_t>;

namespace engine
{
    namespace graphics
    {
        struct GlobalConstants
        {
            Diligent::float4x4 world_view_projection;
            Diligent::float4 viewport_size;
        };
    
        struct GBuffer
        {
            Diligent::RefCntAutoPtr<Diligent::ITexture> color;
            Diligent::RefCntAutoPtr<Diligent::ITexture> depth;
        };

        struct AtmosphereConstants
        {
            // How bright the light is, affects the brightness of the atmosphere
            Diligent::float3 light_intensity;
            // Radius of the planet
            float planet_radius;
            // Radius of the atmosphere
            float atmosphere_radius;
            // Amount rayleigh scattering scatters the colors (for earth: causes the blue atmosphere)
            Diligent::float3 ray_beta;
            // Amount mie scattering scatters colors
            Diligent::float3 mie_beta;
            // Amount of scattering that always occurs, can help make the back side of the atmosphere a bit brighter
            Diligent::float3 ambient_beta;
            // Amount of scattering that that gets absorbed by the atmosphere (Due to things like ozone)
            Diligent::float3 absorption_beta;
            // Direction mie scatters the light in (like a cone). closer to -1 means more towards a single direction
            float g;
        };

        class GraphicsManager
        {
            public:
                GraphicsManager(const std::string& path);

                void initialize(const Diligent::NativeWindow* window);
                void update(double dt);
                void shutdown();
                void resize(uint32_t width, uint32_t height);
                void set_fov(double fov);
                void set_camera_view(Diligent::float4x4 camera_view);

            private:
                void update_g_buffer_();
                void update_(double dt);
                void render_();
                void present_();

                Diligent::float4x4 get_adjusted_projection_matrix_(float fov, float near, float far) const;
                bool create_device_and_swap_chain_metal_(const Diligent::NativeWindow* window);
                void create_swap_chain_metal_(const Diligent::NativeWindow* window);

                void create_sphere_pso_();
                void create_plane_pso_();
                void create_post_process_pso_();

                void render_sphere_();
                void render_plane_();
                void render_post_process_();

                /// Sky
                void create_ambient_sky_light_texture_();
                
                std::string assets_path_;

                /// MARK: - Rendering
                Diligent::IRenderDevice* device_ = nullptr;
                Diligent::IDeviceContext* context_ = nullptr;
                Diligent::ISwapChain* swap_chain_ = nullptr;
                Diligent::SwapChainDesc swap_chain_desc_;
                GBuffer g_buffer_;
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

                Diligent::RefCntAutoPtr<Diligent::IBuffer> global_constants_;
                Diligent::float4x4 camera_view_projection_;

                /// MARK: - Camera
                Diligent::float4x4 camera_view_;
                double fov_ = 60.0;

                /// Post Process
                Diligent::RefCntAutoPtr<Diligent::IPipelineState> post_process_pso_;
                Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> post_process_srb_;

        };
    }
}
