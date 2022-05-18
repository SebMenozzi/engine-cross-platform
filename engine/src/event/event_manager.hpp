#pragma once

#include <functional>
#include <list>
#include <unordered_map>

#include "event_types.hpp"
#include "event.hpp"

namespace engine
{
    namespace event
    {
        class EventManager
        {
            public:
                void add_listener(EventId eventId, std::function<void(Event&)> const& listener)
                {
                    listeners_[eventId].push_back(listener);
                }

                void send_event(Event& event)
                {
                    uint32_t type = event.get_type();

                    for (auto const& listener : listeners_[type])
                        listener(event);
                }

                void send_event(EventId event_id)
                {
                    Event event(event_id);

                    for (auto const& listener : listeners_[event_id])
                        listener(event);
                }
            private:
                std::unordered_map<EventId, std::list<std::function<void(Event&)>>> listeners_;
        };
    }
}