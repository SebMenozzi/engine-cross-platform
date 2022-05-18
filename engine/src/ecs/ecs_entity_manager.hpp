#pragma once

#include <queue>

#include "ecs_types.hpp"

namespace engine
{
    namespace ecs
    {
        class ECSEntityManager
        {
            public:
                ECSEntityManager()
                {
                    // Initialize the queue with all possible entity IDs
                    for (ECSEntity entity = 0; entity < MAX_ENTITIES; ++entity)
                        unused_entity_queue_.push(entity);
                }

                ECSEntity create_entity()
                {
                    assert(nb_entities_ < MAX_ENTITIES && "Too many entities in existence.");

                    // Take an ID from the front of the queue
                    ECSEntity id = unused_entity_queue_.front();
                    unused_entity_queue_.pop();
                    ++nb_entities_;

                    return id;
                }

                void destroy_entity(ECSEntity entity)
                {
                    assert(entity < MAX_ENTITIES && "Entity out of range.");

                    // Invalidate the destroyed entity's mask
                    entity_masks_[entity].reset();

                    // Put the destroyed ID at the back of the queue
                    unused_entity_queue_.push(entity);
                    --nb_entities_;
                }

                void set_mask(ECSEntity entity, ECSMask mask)
                {
                    assert(entity < MAX_ENTITIES && "Entity out of range.");

                    // Put this entity's mask into the array
                    entity_masks_[entity] = mask;
                }

                ECSMask get_mask(ECSEntity entity)
                {
                    assert(entity < MAX_ENTITIES && "Entity out of range.");

                    // Get this entity's mask from the array
                    return entity_masks_[entity];
                }

            private:
                // Queue of unused entity IDs
                std::queue<ECSEntity> unused_entity_queue_{};

                // Array of masks where the index corresponds to the entity ID
                std::array<ECSMask, MAX_ENTITIES> entity_masks_{};

                // Total living entities - used to keep limits on how many exist
                uint32_t nb_entities_{};
        };
    }
}