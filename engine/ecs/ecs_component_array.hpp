#pragma once

#include <array>
#include <cassert>
#include <unordered_map>

#include "ecs_types.hpp"

namespace engine
{
    namespace ecs
    {
        // The one instance of virtual inheritance in the entire implementation.
        // An interface is needed so that the ComponentManager
        // can tell a generic ComponentArray that an entity has been destroyed
        // and that it needs to update its array mappings.
        class ECSComponentArrayInterface
        {
            public:
                virtual ~ECSComponentArrayInterface() = default;
                virtual void entity_destroyed(ECSEntity entity) = 0;
        };

        template<typename T>
        class ECSComponentArray : public ECSComponentArrayInterface
        {
            public:
                void insert(ECSEntity entity, T component)
                {
                    assert(entity_to_index_.find(entity) == entity_to_index_.end() && "Component added to same entity more than once.");

                    // Put new entry at end and update the maps
                    unsigned int index = nb_components_;
                    entity_to_index_[entity] = index;
                    index_to_entity_[index] = entity;
                    components_[index] = component;
                    ++nb_components_;
                }

                void remove(ECSEntity entity)
                {
                    assert(entity_to_index_.find(entity) != entity_to_index_.end() && "Removing non-existent component.");

                    // Copy element at end into deleted element's place to maintain density
                    unsigned int index_removed_entity = entity_to_index_[entity];
                    unsigned int index_last_entity = nb_components_ - 1;
                    components_[index_removed_entity] = components_[index_last_entity];

                    // Update map to point to moved spot
                    ECSEntity entityOfLastElement = index_to_entity_[index_last_entity];
                    entity_to_index_[entityOfLastElement] = index_removed_entity;
                    entity_to_index_[index_removed_entity] = entityOfLastElement;

                    entity_to_index_.erase(entity);
                    entity_to_index_.erase(index_last_entity);

                    --nb_components_;
                }

                T& get(ECSEntity entity)
                {
                    assert(entity_to_index_.find(entity) != entity_to_index_.end() && "Retrieving non-existent component.");

                    // Return a reference to the entity's component
                    return components_[entity_to_index_[entity]];
                }

                void entity_destroyed(ECSEntity entity) override
                {
                    // Remove the entity's component if it existed
                    if (entity_to_index_.find(entity) != entity_to_index_.end())
                        remove(entity);
                }

            private:
                // The packed array of components (of generic type T),
                // set to a specified maximum amount, matching the maximum number
                // of entities allowed to exist simultaneously, so that each entity
                // has a unique spot.
                std::array<T, MAX_ENTITIES> components_;

                // Map from an entity ID to an array index.
                std::unordered_map<ECSEntity, unsigned int> entity_to_index_;

                // Map from an array index to an entity ID.
                std::unordered_map<unsigned int, ECSEntity> index_to_entity_;

                // Total size of valid entries in the array.
                unsigned int nb_components_;
        };
    }
}
