#include "entity.h"

Entity EntityManager::createEntity() {
    Entity e;
    

    if (!freeIds.empty()) {
        e.id = freeIds.back();
        freeIds.pop_back();
    } else {
        e.id = nextEntityId++;
    }
    
    activeEntities.insert(e);
    return e;
}

void EntityManager::destroyEntity(Entity e) {
    if (activeEntities.erase(e) > 0) {
        freeIds.push_back(e.id);
    }
}

std::optional<Entity> EntityManager::getEntityById(uint32_t id) {
    Entity e{ id };
    auto it = activeEntities.find(e);
    
    if (it != activeEntities.end()) {
        return *it;
    }
    return std::nullopt;
}

bool EntityManager::isValid(Entity e) const {
    return activeEntities.find(e) != activeEntities.end();
}

uint32_t EntityManager::getEntityCount() const {
    return activeEntities.size();
}
