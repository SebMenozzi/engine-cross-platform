#pragma once

#include <typeinfo>
#include <memory>
#include <unordered_map>

#include "ecs_types.hpp"
#include "ecs_component_array.hpp"

namespace engine
{
    namespace ecs
    {
        class ECSComponentManager
        {
            public:
                template<typename T>
                void register_component()
                {
                    const char* type_name = typeid(T).name();

                    assert(component_types_.find(type_name) == component_types_.end() && "Registering component type more than once.");

                    // Add this component type to the component type map
                    component_types_.insert({type_name, next_component_type_});

                    // Create a ComponentArray pointer and add it to the component arrays map
                    component_arrays_.insert({type_name, std::make_shared<ECSComponentArray<T>>()});

                    // Increment the value so that the next component registered will be different
                    ++next_component_type_;
                }

                template<typename T>
                ECSComponentType get_component_type()
                {
                    const char* type_name = typeid(T).name();

                    assert(component_types_.find(type_name) != component_types_.end() && "Component not registered before use.");

                    // Return this component's type - used for creating masks
                    return component_types_[type_name];
                }

                template<typename T>
                void add_component(ECSEntity entity, T component)
                {
                    // Add a component to the array for an entity
                    get_component_array_<T>()->insert(entity, component);
                }

                template<typename T>
                void remove_component(ECSEntity entity)
                {
                    // Remove a component from the array for an entity
                    get_component_array_<T>()->remove(entity);
                }

                template<typename T>
                T& get_component(ECSEntity entity)
                {
                    // Get a reference to a component from the array for an entity
                    return get_component_array_<T>()->get(entity);
                }

                void entity_destroyed(ECSEntity entity)
                {
                    // Notify each component array that an entity has been destroyed
                    // If it has a component for that entity, it will remove it
                    for (auto const& pair : component_arrays_)
                    {
                        auto const& component = pair.second;
                        component->entity_destroyed(entity);
                    }
                }

            private:
                // Map from type string pointer to a component type
                std::unordered_map<const char*, ECSComponentType> component_types_{};

                // Map from type string pointer to a component array
                std::unordered_map<const char*, std::shared_ptr<ECSComponentArrayInterface>> component_arrays_{};

                // The component type to be assigned to the next registered component - starting at 0
                ECSComponentType next_component_type_{};

                // Convenience function to get the statically casted pointer to the ComponentArray of type T.
                template<typename T>
                std::shared_ptr<ECSComponentArray<T>> get_component_array_()
                {
                    const char* type_name = typeid(T).name();

                    assert(component_types_.find(type_name) != component_types_.end() && "Component not registered before use.");

                    return std::static_pointer_cast<ECSComponentArray<T>>(component_arrays_[type_name]);
                }
        };
    }
}