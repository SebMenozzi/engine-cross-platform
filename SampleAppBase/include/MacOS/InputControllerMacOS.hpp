#pragma once

namespace Diligent
{
    class InputControllerMacOS : public InputControllerBase
    {
        public:
            enum class MouseButtonEvent
            {
                LMB_Pressed,
                LMB_Released,
                RMB_Pressed,
                RMB_Released
            };
            void OnMouseButtonEvent(MouseButtonEvent Event);

            void OnMouseMove(int MouseX, int MouseY)
            {
                m_MouseState.PosX = static_cast<float>(MouseX);
                m_MouseState.PosY = static_cast<float>(MouseY);
            }

            void OnMouseWheel(float WheelDelta)
            {
                m_MouseState.WheelDelta = WheelDelta;
            }

            void OnKeyPressed(int key);
            void OnKeyReleased(int key);
            void OnFlagsChanged(bool ShiftPressed, bool CtrlPressed, bool AltPressed);

        private:
            void ProcessKeyEvent(int key, bool IsKeyPressed);
    };
}
