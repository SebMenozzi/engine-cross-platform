#pragma once

#include <any>
#include <unordered_map>

#include "event_types.hpp"

namespace engine
{
    namespace event
    {
        class Event
        {
            public:
                Event() = delete;

                explicit Event(EventId type): type_(type)
                {}

                template<typename T>
                void set_parameter(EventId id, T value)
                {
                    data_[id] = value;
                }

                template<typename T>
                T get_parameter(EventId id)
                {
                    return std::any_cast<T>(data_[id]);
                }

                EventId get_type() const
                {
                    return type_;
                }
                
            private:
                EventId type_{};
                std::unordered_map<EventId, std::any> data_{};
        };
    }
}