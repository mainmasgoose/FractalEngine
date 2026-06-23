#include "ComponentManager.h"


#include <atomic>
#include <stdexcept>
#include <algorithm>
#include <cstring>

#include "../../MemoryAlloc/EngineArena.h"

ComponentManager::ComponentManager() {
	cmdMem = std::make_unique<EngineArena>(2 * 1024 * 1024);
}

void ComponentManager::attachComponent(const AttachCMD& cmd) {
	attachComponent(cmd.entity, cmd.compHashId, cmd.data);
}

void ComponentManager::removeComponent(const RemoveCMD& cmd) {
	removeComponent(cmd.entity, cmd.compHashId);
}

void ComponentManager::flushCommands() {
	state.store(ECSState::Syncing);

	for (const auto& cmd : attachQueue) {
		attachComponent(cmd.entity, cmd.compHashId, cmd.data);
	}
	for (const auto& cmd : removeQueue) {
		removeComponent(cmd.entity, cmd.compHashId);
	}

	attachQueue.clear();
	removeQueue.clear();

	if (cmdMem) {
		cmdMem->reset();
	}

	state.store(ECSState::Idle);
}

void ComponentManager::registerComponent(const uint32_t hashId, size_t elementSize, size_t capacity) {
	if (components.find(hashId) != components.end()) return;
	
	size_t alignedElementSize = (elementSize + 15) & ~15;
	// Allocate enough space for data, sparse and dense arrays + some extra
	size_t totalBlockSize = alignedElementSize * capacity + (2 * capacity * sizeof(uint32_t)) + 1024;
	
	EngineArena* arena = std::make_unique<EngineArena>(totalBlockSize).release();
	ECSDomainArenas.push_back(std::unique_ptr<EngineArena>(arena));
	components[hashId] = std::make_unique<ComponentData>(elementSize, capacity, arena);
}

void ComponentManager::resizeComponent(const uint32_t hashId, size_t newCapacity) {
    auto it = components.find(hashId);
    if (it == components.end()) return;
    it->second->resize(newCapacity);
}


void ComponentManager::attachComponentDeferred(Entity e, const uint32_t hashId, void* data, size_t size) {
	if (state.load() == ECSState::Processing) {
		std::lock_guard<std::mutex> lock(cmdLock);
		size_t actualSize = getComponentSize(hashId);
		void* arenaMem = cmdMem->allocate(actualSize, 16);
		std::memset(arenaMem, 0, actualSize);
		std::memcpy(arenaMem, data, size);
		attachQueue.push_back({hashId, e, arenaMem, actualSize});
	} else {
		attachComponent(e, hashId, data);
	}
}

void ComponentManager::removeComponentDeferred(Entity e, const uint32_t hashId) {
	if (state.load() == ECSState::Processing) {
		std::lock_guard<std::mutex> lock(cmdLock);
		removeQueue.push_back({hashId, e});
	} else {
		removeComponent(e, hashId);
	}
}

void* ComponentManager::getComponent(Entity e, const uint32_t hashId) {
	if (state.load() == ECSState::Syncing) {
		throw std::runtime_error("Race condition: tried to read component while syncing");
	}
	auto it = components.find(hashId);
	if (it == components.end()) return nullptr;
	return it->second->get(e);
}

bool ComponentManager::hasComponent(Entity e, const uint32_t hashId) {
	auto it = components.find(hashId);
	if (it == components.end()) return false;
	return it->second->has(e);
}

ComponentData* ComponentManager::getComponentData(const uint32_t hashId) {
	auto it = components.find(hashId);
	if (it == components.end()) return nullptr;
	return it->second.get();
}

void ComponentManager::onComponentAttached(Entity e, uint32_t hashId) {
    for (auto& group : groups) {
        if (contains(group.components, group.count, hashId)) {
            if (hasAll(e, group.components, group.count)) {
                for (size_t i = 0; i < group.count; ++i) {
                    uint32_t compId = group.components[i];
                    components[compId]->moveToGroup(e);
                }
                group.size++;
            }
        }
    }
}

void ComponentManager::onComponentRemoved(Entity e, const uint32_t hashId) {
    for (auto& group : groups) {
        if (contains(group.components, group.count, hashId)) {
            bool wasInGroup = true;
            for (size_t i = 0; i < group.count; ++i) {
                uint32_t compId = group.components[i];
                auto* cd = components[compId].get();
                
                if (cd->sparse[e.id] >= cd->groupedCount) {
                    wasInGroup = false; 
                    break;
                }
            }

            if (wasInGroup) {
                for (size_t i = 0; i < group.count; ++i) {
                    uint32_t compId = group.components[i];
                    auto* cd = components[compId].get();
                    
                    uint32_t currentIdx = cd->sparse[e.id];
                    cd->groupedCount--;
                    
                    cd->swapEntities(currentIdx, (uint32_t)cd->groupedCount);
                }
                group.size--;
            }
        }
    }
}
void ComponentManager::registerGroup(const uint32_t* componentHashIds, size_t count) {
    Group newGroup;
    newGroup.count = count;
    newGroup.size = 0;
    newGroup.components = (uint32_t*)malloc(count * sizeof(uint32_t));
    std::memcpy(newGroup.components, componentHashIds, count * sizeof(uint32_t));
    
    groups.push_back(newGroup);
}

void ComponentManager::attachComponent(Entity e, const uint32_t hashId, void* data) {
	auto it = components.find(hashId);
	if (it == components.end()) throw std::runtime_error("Component not registered");
	it->second->attach(e, data);
	onComponentAttached(e, hashId);
}

void ComponentManager::removeComponent(Entity e, const uint32_t hashId) {
	onComponentRemoved(e, hashId);
	auto it = components.find(hashId);
	if (it == components.end()) return;
	it->second->remove(e);
}

bool ComponentManager::contains(const uint32_t* array, size_t count, uint32_t value) {
    for (size_t i = 0; i < count; ++i) {
        if (array[i] == value) return true;
    }
    return false;
}
bool ComponentManager::hasAll(Entity e, const uint32_t* comps, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (!hasComponent(e, comps[i])) return false;
    }
    return true;
}
size_t ComponentManager::getComponentSize(const uint32_t hashId) {
	auto it = components.find(hashId);
	if (it == components.end()) return 0;
	return it->second->elementSize;
}
size_t ComponentManager::getGroupSize(const uint32_t* compNames, size_t count) {
    for (const auto& group : groups) {
        // Comparing arrays by content
        if (group.count == count && 
            std::memcmp(group.components, compNames, count * sizeof(uint32_t)) == 0) {
            return group.size;
        }
    }
    return 0;
}
std::atomic_bool& ComponentManager::getComponentLock(const uint32_t hashId) {
	auto it = componentLocks.find(hashId);
	if (it == componentLocks.end()) {
		componentLocks[hashId] = false;
	}
	return componentLocks[hashId];
}
void ComponentManager::setComponentLock(const uint32_t hashId, bool lock) {
    getComponentLock(hashId).store(lock, std::memory_order_release);
}
void* ComponentManager::getRawPtr(const uint32_t hashId, bool lock) {
	auto it = components.find(hashId);
	if (it == components.end()) return nullptr;
    if (lock) {
        getComponentLock(hashId).store(true, std::memory_order_release);
    }
	return it->second->getRawPtr();
}
