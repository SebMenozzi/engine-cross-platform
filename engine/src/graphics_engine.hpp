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

#include "EngineFactory.h"
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "BasicMath.hpp" // float4x4
#include "MapHelper.hpp"
#include "Timer.hpp"

using Buffer = std::vector<uint8_t>;

namespace Diligent
{
    class ImGuiImplDiligent;

    class GraphicsEngine
    {
        public:
            GraphicsEngine();

            void initialize(const NativeWindow* window);
            void update();
            void stop();
            void resize(uint32_t width, uint32_t height);

        private:
            IRenderDevice* device_ = nullptr;
            IDeviceContext* context_ = nullptr;
            ISwapChain* swap_chain_ = nullptr;
            SwapChainDesc swap_chain_desc_;

            bool vsync_enabled_ = false;
            bool quit_ = false;

            RefCntAutoPtr<IPipelineState> pso_;
            RefCntAutoPtr<IShaderResourceBinding> srb_;
            RefCntAutoPtr<IBuffer> cube_vertex_buffer_;
            RefCntAutoPtr<IBuffer> cube_index_buffer_;
            RefCntAutoPtr<IBuffer> vertices_constants_;
            float4x4 world_view_projection_matrix_;

            Timer timer_;
            double last_time_ = 0;

            void update_();
            void render_();
            void present_();

            float4x4 get_adjusted_projection_matrix_(float fov, float near, float far) const;
            bool create_device_and_swap_chain_metal_(const NativeWindow* window);
            void create_swap_chain_metal_(const NativeWindow* window);
            std::optional<Buffer> read_file_(const std::filesystem::path &path);
            void create_pipeline_state_();
            void create_vertex_buffer_();
            void create_index_buffer_();
    };
}
