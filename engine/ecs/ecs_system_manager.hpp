#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>

#include "ecs_types.hpp"
#include "ecs_system.hpp"

namespace engine
{
    namespace ecs
    {
        class ECSSystemManager
        {
            public:
                template<typename T>
                std::shared_ptr<T> register_system()
                {
                    const char* type_name = typeid(T).name();

                    assert(systems_.find(type_name) == systems_.end() && "Registering system more than once.");

                    // Create a pointer to the system and return it so it can be used externally
                    auto system = std::make_shared<T>();
                    systems_.insert({type_name, system});

                    return system;
                }

                template<typename T>
                void set_mask(ECSMask mask)
                {
                    const char* type_name = typeid(T).name();

                    assert(systems_.find(type_name) != systems_.end() && "System used before registered.");

                    // Set the mask for this system
                    masks_.insert({type_name, mask});
                }

                void entity_destroyed(ECSEntity entity)
                {
                    // Erase a destroyed entity from all system lists
                    // mEntities is a set so no check needed
                    for (auto const& pair : systems_)
                    {
                        auto const& system = pair.second;
                        system->entities_.erase(entity);
                    }
                }

                void entity_mask_changed(ECSEntity entity, ECSMask entity_mask)
                {
                    // Notify each system that an entity's mask changed
                    for (auto const& pair : systems_)
                    {
                        auto const& type = pair.first;
                        auto const& system = pair.second;
                        auto const& system_mask = masks_[type];

                        // Entity mask matches system mask - insert into set
                        if ((entity_mask & system_mask) == system_mask)
                            system->entities_.insert(entity);
                        // Entity mask does not match system mask - erase from set
                        else
                            system->entities_.erase(entity);
                    }
                }

            private:
                // Map from system type string pointer to a mask
                std::unordered_map<const char*, ECSMask> masks_{};

                // Map from system type string pointer to a system pointer
                std::unordered_map<const char*, std::shared_ptr<ECSSystem>> systems_{};
        };
    }
}