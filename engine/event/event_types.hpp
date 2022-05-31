#pragma once

#include "utils_hash.hpp"

namespace engine
{
    namespace event
    {
        /// MARK: - Event types

        using EventId = std::uint32_t;
        using EventParameterId = std::uint32_t;

        /// MARK: - Event listener macros

        #define EVENT_METHOD_LISTENER(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1)
        #define EVENT_FUNCTION_LISTENER(EventType, Listener) EventType, std::bind(&Listener, std::placeholders::_1)

        /// MARK: - Events

        const EventId QUIT = "event::quit"_hash;
        const EventId RESIZE = "event::resize"_hash;
        const EventId INPUT = "event::input"_hash;
        const EventId MOUSE_POSITION = "events::mouse_position"_hash;
        const EventId SCROLL_OFFSET = "events::scroll_offset"_hash;
        const EventId CAMERA_ANGLES = "events::camera_angles"_hash;

        /// MARK: - Event parameters

        namespace resize
        {
            const EventParameterId WIDTH = "event::resize::width"_hash;
            const EventParameterId HEIGHT = "event::resize::height"_hash;
        }

        namespace input
        {
            const EventParameterId PARAMETER = "event::input::parameter"_hash;
        }

        namespace mouse_position
        {
            const EventParameterId X = "event::mouse_position::x"_hash;
            const EventParameterId Y = "event::mouse_position::y"_hash;
        }

        namespace scroll_offset
        {
            const EventParameterId X = "event::scroll_offset::x"_hash;
            const EventParameterId Y = "event::scroll_offset::y"_hash;
        }

        namespace camera_angles
        {
            const EventParameterId PITCH = "event::camera_angles::pitch"_hash;
            const EventParameterId YAW = "event::camera_angles::yaw"_hash;
            const EventParameterId ROLL = "event::camera_angles::roll"_hash;
        }
    }
}