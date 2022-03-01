#pragma once

#include <memory>
#include <mutex>

namespace Diligent
{
    class InputControllerUWP
    {
        public:
            class SharedControllerState : private InputControllerBase
            {
            public:
                MouseState GetMouseState()
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    return InputControllerBase::GetMouseState();
                }

                INPUT_KEY_STATE_FLAGS GetKeyState(InputKeys Key)
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    return InputControllerBase::GetKeyState(Key);
                }

                bool IsKeyDown(InputKeys Key)
                {
                    return (GetKeyState(Key) & INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0;
                }

                void ClearState()
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    InputControllerBase::ClearState();
                }

                void OnKeyDown(InputKeys Key)
                {
                    std::lock_guard<std::mutex> lock(mtx);

                    auto& keyState = m_Keys[static_cast<size_t>(Key)];
                    keyState &= ~INPUT_KEY_STATE_FLAG_KEY_WAS_DOWN;
                    keyState |= INPUT_KEY_STATE_FLAG_KEY_IS_DOWN;
                }

                void OnKeyUp(InputKeys Key)
                {
                    std::lock_guard<std::mutex> lock(mtx);

                    auto& keyState = m_Keys[static_cast<size_t>(Key)];
                    keyState &= ~INPUT_KEY_STATE_FLAG_KEY_IS_DOWN;
                    keyState |= INPUT_KEY_STATE_FLAG_KEY_WAS_DOWN;
                }

                void MouseButtonPressed(MouseState::BUTTON_FLAGS Flags)
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    m_MouseState.ButtonFlags |= Flags;
                }

                void MouseButtonReleased(MouseState::BUTTON_FLAGS Flags)
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    m_MouseState.ButtonFlags &= ~Flags;
                }

                void SetMousePose(float x, float y)
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    m_MouseState.PosX = x;
                    m_MouseState.PosY = y;
                }

                void SetMouseWheelDetlta(float w)
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    m_MouseState.WheelDelta = w;
                }

            private:
                std::mutex mtx;
            };

            MouseState GetMouseState()
            {
                return m_SharedState->GetMouseState();
            }

            INPUT_KEY_STATE_FLAGS GetKeyState(InputKeys Key) const
            {
                return m_SharedState->GetKeyState(Key);
            }

            bool IsKeyDown(InputKeys Key) const
            {
                return m_SharedState->IsKeyDown(Key);
            }

            std::shared_ptr<SharedControllerState> GetSharedState()
            {
                return m_SharedState;
            }

            void ClearState()
            {
                m_SharedState->ClearState();
            }

        private:
            std::shared_ptr<SharedControllerState> m_SharedState{new SharedControllerState};
    };
}
