//  ---------------------------------------------------------------------------
//
//  @file       TwEventWinRT.h
//  @brief      Helper: 
//              translate and re-send mouse and keyboard events 
//              from Windows Runtime message proc to AntTweakBar
//  
//  @author     Egor Yusov
//  @date       2015/06/30
//  @note       This file is not part of the AntTweakBar library because
//              it is not recommended to pack Windows Runtime extensions 
//              into a static library
//  ---------------------------------------------------------------------------
#pragma once

#ifdef _WIN64
#define PARAM_INT _int64
#else
#define PARAM_INT int
#endif

ref class ImguiUWPEventHelper
{
internal:
    static ImguiUWPEventHelper^ ImguiUWPEventHelper::Create(_In_ Windows::UI::Core::CoreWindow^ window);

    ImguiUWPEventHelper(_In_ Windows::UI::Core::CoreWindow^ window);

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

    virtual void ShowCursor();
    virtual void HideCursor();

protected private:

    void UpdateImguiMouseProperties(_In_ Windows::UI::Core::PointerEventArgs^ args);
    void UpdateKeyStates(_In_ Windows::UI::Core::KeyEventArgs^ args, bool IsDown);
};
