#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "EngineFactory.h"
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Image.h"
#include "BasicMath.hpp" // float4x4
#include "MapHelper.hpp"
#include "Timer.hpp"

#include "CrossPlatformGraphicsEngine.hpp"

using Buffer = std::vector<uint8_t>;

namespace Diligent
{
    class ImGuiImplDiligent;

    class GraphicsEngine : public CrossPlatformGraphicsEngine
    {
        public:
            virtual void start() override;
            virtual void stop() override;

        protected:
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

            void initialize_diligent_engine(const NativeWindow* window);

        private:
            void update();
            void render();
            void present();

            float4x4 get_adjusted_projection_matrix(float fov, float near, float far) const;
            bool create_device_and_swap_chain_metal(const NativeWindow* window);
            void create_swap_chain_metal(const NativeWindow* window);
            std::optional<Buffer> read_file(const std::filesystem::path &path);
            void create_pipeline_state();
            void create_vertex_buffer();
            void create_index_buffer();
    };
}
