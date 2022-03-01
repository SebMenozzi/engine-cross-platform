#pragma once

#include <vector>

#include "EngineFactory.h"
#include "RefCntAutoPtr.hpp"
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "InputController.hpp"
#include "BasicMath.hpp"

namespace Diligent
{
    class ImGuiImplDiligent;

    struct SampleInitInfo
    {
        IEngineFactory*    pEngineFactory  = nullptr;
        IRenderDevice*     pDevice         = nullptr;
        IDeviceContext**   ppContexts      = nullptr;
        Uint32             NumImmediateCtx = 1;
        Uint32             NumDeferredCtx  = 0;
        ISwapChain*        pSwapChain      = nullptr;
        ImGuiImplDiligent* pImGui          = nullptr;
    };

    class SampleBase
    {
        public:
            virtual ~SampleBase() {}

            struct ModifyEngineInitInfoAttribs
            {
                IEngineFactory* const    pFactory;
                const RENDER_DEVICE_TYPE DeviceType;

                EngineCreateInfo& EngineCI;
                SwapChainDesc&    SCDesc;
            };
            virtual void ModifyEngineInitInfo(const ModifyEngineInitInfoAttribs& Attribs);

            virtual void Initialize(const SampleInitInfo& InitInfo) = 0;

            virtual void Render() = 0;
            virtual void Update(double CurrTime, double ElapsedTime) = 0;
            virtual void PreWindowResize() {}
            virtual void WindowResize(Uint32 Width, Uint32 Height) {}
            virtual bool HandleNativeMessage(const void* pNativeMsgData) { return false; }

            virtual const Char* GetSampleName() const { return "Engine"; }
            virtual void ProcessCommandLine(const char* CmdLine) {}

            InputController& GetInputController()
            {
                return m_InputController;
            }

            void ResetSwapChain(ISwapChain* pNewSwapChain)
            {
                m_pSwapChain = pNewSwapChain;
            }

        protected:
            // Returns projection matrix adjusted to the current screen orientation
            float4x4 GetAdjustedProjectionMatrix(float FOV, float NearPlane, float FarPlane) const;

            // Returns pretransform matrix that matches the current screen rotation
            float4x4 GetSurfacePretransformMatrix(const float3& f3CameraViewAxis) const;

            RefCntAutoPtr<IEngineFactory>              m_pEngineFactory;
            RefCntAutoPtr<IRenderDevice>               m_pDevice;
            RefCntAutoPtr<IDeviceContext>              m_pImmediateContext;
            std::vector<RefCntAutoPtr<IDeviceContext>> m_pDeferredContexts;
            RefCntAutoPtr<ISwapChain>                  m_pSwapChain;
            ImGuiImplDiligent*                         m_pImGui = nullptr;

            float  m_fSmoothFPS         = 0;
            double m_LastFPSTime        = 0;
            Uint32 m_NumFramesRendered  = 0;
            Uint32 m_CurrentFrameNumber = 0;

            InputController m_InputController;
    };

    inline void SampleBase::Update(double CurrTime, double ElapsedTime)
    {
        ++m_NumFramesRendered;
        ++m_CurrentFrameNumber;
        static const double dFPSInterval = 0.5;
        if (CurrTime - m_LastFPSTime > dFPSInterval)
        {
            m_fSmoothFPS        = static_cast<float>(m_NumFramesRendered / (CurrTime - m_LastFPSTime));
            m_NumFramesRendered = 0;
            m_LastFPSTime       = CurrTime;
        }
    }

    inline void SampleBase::Initialize(const SampleInitInfo& InitInfo)
    {
        m_pEngineFactory    = InitInfo.pEngineFactory;
        m_pDevice           = InitInfo.pDevice;
        m_pSwapChain        = InitInfo.pSwapChain;
        m_pImmediateContext = InitInfo.ppContexts[0];
        m_pDeferredContexts.resize(InitInfo.NumDeferredCtx);
        for (Uint32 ctx = 0; ctx < InitInfo.NumDeferredCtx; ++ctx)
            m_pDeferredContexts[ctx] = InitInfo.ppContexts[InitInfo.NumImmediateCtx + ctx];
        m_pImGui = InitInfo.pImGui;
    }

    extern SampleBase* CreateSample();
}
