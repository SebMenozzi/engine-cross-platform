#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>

#include "PlatformDefinitions.h"
#include "Errors.hpp"
#if METAL_SUPPORTED
#include "EngineFactoryMtl.h"
#endif
#include "EngineFactory.h"
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "BasicMath.hpp" // float4x4
#include "MapHelper.hpp"
#include "TextureUtilities.h"
#include "GraphicsUtilities.h" // CreateUniformBuffer
#include "Timer.hpp"

#include "cube.hpp"
#include "plane.hpp"

using Buffer = std::vector<uint8_t>;

namespace engine
{
    class GraphicsEngine
    {
        public:
            GraphicsEngine();

            void initialize(const Diligent::NativeWindow* window);
            void update();
            void stop();
            void resize(uint32_t width, uint32_t height);

        private:
            Diligent::IRenderDevice* device_ = nullptr;
            Diligent::IDeviceContext* context_ = nullptr;
            Diligent::ISwapChain* swap_chain_ = nullptr;
            Diligent::SwapChainDesc swap_chain_desc_;

            bool vsync_enabled_ = false;
            bool quit_ = false;

            /// MARK: - Cube

            Diligent::RefCntAutoPtr<Diligent::IPipelineState> cube_pso_;
            Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> cube_srb_;

            Diligent::RefCntAutoPtr<Diligent::ITextureView> cube_texture_srv_;
            Diligent::RefCntAutoPtr<Diligent::IBuffer> cube_vertex_buffer_;
            Diligent::RefCntAutoPtr<Diligent::IBuffer> cube_index_buffer_;

            /// MARK: - Plane
            
            Diligent::RefCntAutoPtr<Diligent::IPipelineState> plane_pso_;
            Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> plane_srb_;

            Diligent::RefCntAutoPtr<Diligent::IBuffer> plane_vertex_buffer_;
            Diligent::RefCntAutoPtr<Diligent::IBuffer> plane_index_buffer_;

            Diligent::RefCntAutoPtr<Diligent::IBuffer> vertices_constants_;
            Diligent::float4x4 world_view_projection_matrix_;

            Diligent::Timer timer_;
            double last_time_ = 0;

            void update_();
            void render_();
            void present_();

            Diligent::float4x4 get_adjusted_projection_matrix_(float fov, float near, float far) const;
            bool create_device_and_swap_chain_metal_(const Diligent::NativeWindow* window);
            void create_swap_chain_metal_(const Diligent::NativeWindow* window);
            std::optional<Buffer> read_file_(const std::filesystem::path &path);
            void create_cube_pso_();
            void create_plane_pso_();
            void render_cube_();
            void render_plane_();
    };
}
