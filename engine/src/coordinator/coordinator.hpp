#pragma once

#include <memory>

#include "ecs_types.hpp"
#include "ecs_component_manager.hpp"
#include "ecs_entity_manager.hpp"
#include "ecs_system_manager.hpp"

#include "event.hpp"
#include "event_manager.hpp"

namespace engine
{
    class Coordinator
    {
        public:
            void init()
            {
                // Create pointers to each manager
                component_manager_ = std::make_unique<ecs::ECSComponentManager>();
                entity_manager_ = std::make_unique<ecs::ECSEntityManager>();
                system_manager_ = std::make_unique<ecs::ECSSystemManager>();
                event_manager_ = std::make_unique<event::EventManager>();
            }

            /// MARK: - Entity methods

            ecs::ECSEntity create_entity()
            {
                return entity_manager_->create_entity();
            }

            void destroy_entity(ecs::ECSEntity entity)
            {
                entity_manager_->destroy_entity(entity);
                component_manager_->entity_destroyed(entity);
                system_manager_->entity_destroyed(entity);
            }

            /// MARK: - Component methods

            template<typename T>
            void register_component()
            {
                component_manager_->register_component<T>();
            }

            template<typename T>
            void add_component(ecs::ECSEntity entity, T component)
            {
                component_manager_->add_component<T>(entity, component);

                auto mask = entity_manager_->get_mask(entity);
                mask.set(component_manager_->get_component_type<T>(), true);
                entity_manager_->set_mask(entity, mask);

                system_manager_->entity_mask_changed(entity, mask);
            }

            template<typename T>
            void remove_component(ecs::ECSEntity entity)
            {
                component_manager_->remove_component<T>(entity);

                auto mask = entity_manager_->get_mask(entity);
                mask.set(component_manager_->get_component_type<T>(), false);
                entity_manager_->set_mask(entity, mask);

                system_manager_->entity_mask_changed(entity, mask);
            }

            template<typename T>
            T& get_component(ecs::ECSEntity entity)
            {
                return component_manager_->get_component<T>(entity);
            }

            template<typename T>
            ecs::ECSComponentType get_component_type()
            {
                return component_manager_->get_component_type<T>();
            }

            /// MARK: - System methods

            template<typename T>
            std::shared_ptr<T> register_system()
            {
                return system_manager_->register_system<T>();
            }

            template<typename T>
            void set_system_mask(ecs::ECSMask mask)
            {
                system_manager_->set_mask<T>(mask);
            }

            /// MARK: - Event methods

            void add_event_listener(event::EventId event_id, std::function<void(event::Event&)> const& listener)
            {
                event_manager_->add_listener(event_id, listener);
            }

            void send_event(event::Event& event)
            {
                event_manager_->send_event(event);
            }

            void send_event(event::EventId event_id)
            {
                event_manager_->send_event(event_id);
            }

        private:
            std::unique_ptr<ecs::ECSComponentManager> component_manager_;
            std::unique_ptr<ecs::ECSEntityManager> entity_manager_;
            std::unique_ptr<ecs::ECSSystemManager> system_manager_;
            std::unique_ptr<event::EventManager> event_manager_;
    };
}