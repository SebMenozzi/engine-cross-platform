#pragma once

#include "BasicTypes.h"
#include "FlagEnum.h"

namespace Diligent
{
    struct MouseState
    {
        enum BUTTON_FLAGS : Uint8
        {
            BUTTON_FLAG_NONE   = 0x00,
            BUTTON_FLAG_LEFT   = 0x01,
            BUTTON_FLAG_MIDDLE = 0x02,
            BUTTON_FLAG_RIGHT  = 0x04,
            BUTTON_FLAG_WHEEL  = 0x08
        };

        Float32      PosX        = -1;
        Float32      PosY        = -1;
        BUTTON_FLAGS ButtonFlags = BUTTON_FLAG_NONE;
        Float32      WheelDelta  = 0;
    };
    DEFINE_FLAG_ENUM_OPERATORS(MouseState::BUTTON_FLAGS)

    enum class InputKeys
    {
        Unknown = 0,
        MoveLeft,
        MoveRight,
        MoveForward,
        MoveBackward,
        MoveUp,
        MoveDown,
        Reset,
        ControlDown,
        ShiftDown,
        AltDown,
        ZoomIn,
        ZoomOut,
        TotalKeys
    };

    enum INPUT_KEY_STATE_FLAGS : Uint8
    {
        INPUT_KEY_STATE_FLAG_KEY_NONE     = 0x00,
        INPUT_KEY_STATE_FLAG_KEY_IS_DOWN  = 0x01,
        INPUT_KEY_STATE_FLAG_KEY_WAS_DOWN = 0x80
    };
    DEFINE_FLAG_ENUM_OPERATORS(INPUT_KEY_STATE_FLAGS)

    class InputControllerBase
    {
        public:
            const MouseState& GetMouseState() const
            {
                return m_MouseState;
            }

            INPUT_KEY_STATE_FLAGS GetKeyState(InputKeys Key) const
            {
                return m_Keys[static_cast<size_t>(Key)];
            }

            bool IsKeyDown(InputKeys Key) const
            {
                return (GetKeyState(Key) & INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0;
            }

            void ClearState()
            {
                m_MouseState.WheelDelta = 0;

                for (Uint32 i = 0; i < static_cast<Uint32>(InputKeys::TotalKeys); ++i)
                {
                    auto& KeyState = m_Keys[i];
                    if (KeyState & INPUT_KEY_STATE_FLAG_KEY_WAS_DOWN)
                    {
                        KeyState &= ~INPUT_KEY_STATE_FLAG_KEY_WAS_DOWN;
                    }
                }
            }

        protected:
            MouseState            m_MouseState;
            INPUT_KEY_STATE_FLAGS m_Keys[static_cast<size_t>(InputKeys::TotalKeys)] = {};
    };

}

// clang-format off
#if PLATFORM_WIN32
#include "Win32/InputControllerWin32.hpp"
namespace Diligent
{
    using InputController = InputControllerWin32;
}
#elif PLATFORM_UNIVERSAL_WINDOWS
#include "UWP/InputControllerUWP.hpp"
namespace Diligent
{
    using InputController = InputControllerUWP;
}
#elif PLATFORM_MACOS
#include "MacOS/InputControllerMacOS.hpp"
namespace Diligent
{
    using InputController = InputControllerMacOS;
}
#elif PLATFORM_LINUX
#include "Linux/InputControllerLinux.hpp"
namespace Diligent
{
    using InputController = InputControllerLinux;
}
#else
    namespace Diligent
    {
        class DummyInputController
        {
        public:
            const MouseState& GetMouseState()const{return m_MouseState;}

            INPUT_KEY_STATE_FLAGS GetKeyState(InputKeys Key)const{return INPUT_KEY_STATE_FLAG_KEY_NONE;}

            bool IsKeyDown(InputKeys Key)const{return false;}

            void ClearState(){}

        private:
            MouseState m_MouseState;
        };
        using InputController = DummyInputController;
    }
#endif
// clang-format on
