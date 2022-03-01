#include <queue>
#include <mutex>
#import <Cocoa/Cocoa.h>
#include "SampleApp.hpp"
#include "ImGuiImplMacOS.hpp"

namespace Diligent
{

class SampleAppMacOS final : public SampleApp
{
public:
    SampleAppMacOS()
    {
        m_DeviceType = RENDER_DEVICE_TYPE_GL;
    }

    virtual void Initialize(void* view, RenderMode Mode)override final
    {
        switch (Mode)
        {
            case RenderMode::OpenGL:
                m_DeviceType = RENDER_DEVICE_TYPE_GL;
                break;

            case RenderMode::MoltenVK:
                VERIFY_EXPR(view != nullptr);
                m_DeviceType = RENDER_DEVICE_TYPE_VULKAN;
                break;

            case RenderMode::Metal:
                VERIFY_EXPR(view != nullptr);
                m_DeviceType = RENDER_DEVICE_TYPE_METAL;
                break;

            default:
                UNEXPECTED("Unexpected render mode");
        }

        MacOSNativeWindow MacWindow{view};
        InitializeDiligentEngine(&MacWindow);
        const auto& SCDesc = m_pSwapChain->GetDesc();

        m_pImGui.reset(new ImGuiImplMacOS((__bridge NSView*)view, m_pDevice, SCDesc.ColorBufferFormat, SCDesc.DepthBufferFormat));
        InitializeSample();

        if (m_DeviceType == RENDER_DEVICE_TYPE_METAL)
        {
            // In Metal, FinishFrame must be called from the same thread
            // that issued rendering commands. On MacOS, however, rendering
            // happens in DisplayLinkCallback which is called from some other
            // thread. To avoid issues with autorelease pool, we have to pop
            // it now by calling FinishFrame.
            GetImmediateContext()->Flush();
            GetImmediateContext()->FinishFrame();
        }
    }

    virtual void Render()override
    {
        std::lock_guard<std::mutex> lock(AppMutex);

        GetImmediateContext()->SetRenderTargets(0, nullptr, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        SampleApp::Render();
    }

    virtual void Update(double CurrTime, double ElapsedTime)override
    {
        std::lock_guard<std::mutex> lock(AppMutex);
        SampleApp::Update(CurrTime, ElapsedTime);
    }

    virtual void WindowResize(int width, int height)override
    {
        std::lock_guard<std::mutex> lock(AppMutex);
        SampleApp::WindowResize(width, height);
    }

    virtual void Present()override
    {
        std::lock_guard<std::mutex> lock(AppMutex);
        SampleApp::Present();
    }

    virtual void HandleOSXEvent(void* _event, void* _view)override final
    {
        auto* event = (__bridge NSEvent*)_event;

        switch(event.type)
        {
            case NSEventTypeLeftMouseDown:
            case NSEventTypeRightMouseDown:
            case NSEventTypeLeftMouseUp:
            case NSEventTypeRightMouseUp:
            case NSEventTypeOtherMouseDown:
            case NSEventTypeOtherMouseUp:
            case NSEventTypeMouseMoved:
            case NSEventTypeLeftMouseDragged:
            case NSEventTypeRightMouseDragged:
            case NSEventTypeKeyDown:
            case NSEventTypeKeyUp:
            case NSEventTypeFlagsChanged:
            case NSEventTypeScrollWheel:
                break;
            
            default:
                return;
        }

        std::lock_guard<std::mutex> lock(AppMutex);
        auto& inputController = m_TheSample->GetInputController();

        auto HandleKeyEvent = [](NSEvent* event, InputController& inputController)
        {
            NSString* str = [event characters];
            int len = (int)[str length];
            for (int i = 0; i < len; i++)
            {
                int c = [str characterAtIndex:i];
                int key = 0;
                switch(c)
                {
                    case NSLeftArrowFunctionKey:  key = 260; break;
                    case NSRightArrowFunctionKey: key = 262; break;
                    case 0x7F:                    key = '\b'; break;
                    default:                      key = c;
                }
                if (event.type == NSEventTypeKeyDown)
                    inputController.OnKeyPressed(key);
                else if (event.type == NSEventTypeKeyUp)
                    inputController.OnKeyReleased(key);
                else
                    UNEXPECTED("Unexpected event type");
            }
        };

        auto* view  = (__bridge NSView*)_view;
        if (!static_cast<ImGuiImplMacOS*>(m_pImGui.get())->HandleOSXEvent(event, view))
        {
            if (event.type == NSEventTypeLeftMouseDown ||
                event.type == NSEventTypeRightMouseDown)
            {
                auto ControllerEvent = event.type == NSEventTypeLeftMouseDown ?
                            InputController::MouseButtonEvent::LMB_Pressed :
                            InputController::MouseButtonEvent::RMB_Pressed;
                inputController.OnMouseButtonEvent(ControllerEvent);
            }

            if (event.type == NSEventTypeKeyDown)
            {
                HandleKeyEvent(event, inputController);
            }

            if (event.type == NSEventTypeScrollWheel)
            {
                double wheel_dx = 0.0;
                double wheel_dy = 0.0;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
                if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
                {
                    wheel_dx = [event scrollingDeltaX];
                    wheel_dy = [event scrollingDeltaY];
                    if ([event hasPreciseScrollingDeltas])
                    {
                        wheel_dx *= 0.1;
                        wheel_dy *= 0.1;
                    }
                }
                else
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/
                {
                    wheel_dx = [event deltaX];
                    wheel_dy = [event deltaY];
                }

                inputController.OnMouseWheel(static_cast<float>(wheel_dy * 0.1));
            }
        }

        if (event.type == NSEventTypeLeftMouseUp ||
            event.type == NSEventTypeRightMouseUp)
        {
            auto ControllerEvent =
                    event.type == NSEventTypeLeftMouseUp ?
                        InputController::MouseButtonEvent::LMB_Released :
                        InputController::MouseButtonEvent::RMB_Released;
            inputController.OnMouseButtonEvent(ControllerEvent);
        }

        if (event.type == NSEventTypeLeftMouseDragged  ||
            event.type == NSEventTypeRightMouseDragged ||
            event.type == NSEventTypeMouseMoved)
        {
            NSRect viewRectPoints = [view bounds];
            NSRect viewRectPixels = [view convertRectToBacking:viewRectPoints];
            NSPoint curPoint = [view convertPoint:[event locationInWindow] fromView:nil];
            curPoint = [view convertPointToBacking:curPoint];
            inputController.OnMouseMove(curPoint.x, viewRectPixels.size.height-1 - curPoint.y);
        }

        if (event.type == NSEventTypeKeyUp)
        {
            HandleKeyEvent(event, inputController);
        }

        if (event.type ==  NSEventTypeFlagsChanged)
        {
            auto modifierFlags = [event modifierFlags];
            {
                inputController.OnFlagsChanged(modifierFlags & NSEventModifierFlagShift,
                                               modifierFlags & NSEventModifierFlagControl,
                                               modifierFlags & NSEventModifierFlagOption);
            }
        }
    }

private:
    // Render functions are called from high-priority Display Link thread,
    // so all methods must be protected by mutex
    std::mutex AppMutex;
};

NativeAppBase* CreateApplication()
{
    return new SampleAppMacOS;
}

}
