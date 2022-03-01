#pragma once

#include "InputController.hpp"

namespace Diligent
{
 
ref class InputControllerEventHandlerUWP
{
internal:
    static InputControllerEventHandlerUWP^ InputControllerEventHandlerUWP::Create(
                                   _In_ Windows::UI::Core::CoreWindow^ window,
                                   std::shared_ptr<InputControllerUWP::SharedControllerState> SharedState);

    InputControllerEventHandlerUWP(_In_ Windows::UI::Core::CoreWindow^ window,
                                   std::shared_ptr<InputControllerUWP::SharedControllerState> SharedState);

protected:
    void OnPointerPressed(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::PointerEventArgs^ args
        );
    void OnPointerMoved(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::PointerEventArgs^ args
        );
    void OnPointerReleased(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::PointerEventArgs^ args
        );
    void OnPointerExited(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::PointerEventArgs^ args
        );
    void OnKeyDown(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::KeyEventArgs^ args
        );
    void OnKeyUp(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::KeyEventArgs^ args
        );
    void OnMouseMoved(
        _In_ Windows::Devices::Input::MouseDevice^ mouseDevice,
        _In_ Windows::Devices::Input::MouseEventArgs^ args
        );
    void OnPointerWheelChanged(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::PointerEventArgs^ args
        );

    //virtual void ShowCursor();
    //virtual void HideCursor();

protected private:

    std::shared_ptr<InputControllerUWP::SharedControllerState> m_SharedState;

    float m_LastMousePosX = -1;
    float m_LastMousePosY = -1;
};

}
