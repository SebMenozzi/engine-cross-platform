#pragma once

#include "coordinator.hpp"
#include "graphics_manager.hpp"

#include "event.hpp"
#include "event_types.hpp"

#include "gravity.hpp"
#include "camera.hpp"
#include "rigid_body.hpp"
#include "transform.hpp"

#include "camera_control_system.hpp"
#include "physics_system.hpp"

namespace engine
{
    class Engine
    {
        public:
            void init(Diligent::NativeWindow native_window);
            void update(double dt);
            void shutdown();
            bool should_quit();
            void send_event(event::Event& event);
            void send_event(event::EventId event_id);
    };
}