#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <cstring>
#include <string>

#include "ComponentData.h"
#include "hash/hash.h"
#include "ankerl/unordered_dense.h"
enum class ECSState {
    Idle,
    Processing,
    Syncing
};
struct AttachCMD {
    uint32_t compHashId;
    Entity entity;
    void* data;
    size_t size;
};
struct RemoveCMD {
    uint32_t compHashId;
    Entity entity;
};

struct Group {
    uint32_t* components;
    size_t count;
    size_t size;
};
class EngineArena; // forward

/**
 * ComponentManager (Component Manager) - Core Component.
 * Manages component lifecycles, registration, and storage in the ECS.
 */
class ComponentManager {
public:
    /**
     * Initializes a new instance of the ComponentManager.
     */
    ComponentManager();
    /**
     * Destroys the ComponentManager and releases resources.
     */
    ~ComponentManager() = default;

    /**
     * Attaches a component to an entity based on the provided command.
     * 
     * @param cmd Command containing attachment details.
     */
    void attachComponent(const AttachCMD& cmd);
    /**
     * Removes a component from an entity based on the provided command.
     * 
     * @param cmd Command containing removal details.
     */
    void removeComponent(const RemoveCMD& cmd);
    /**
     * Processes and flushes all queued component attachment and removal commands.
     */
    void flushCommands();

    /**
     * Registers a new component type with the manager.
     * 
     * @param hashId Unique identifier for the component type.
     * @param elementSize Size of a single component instance.
     * @param capacity Initial capacity for the component storage.
     */
    void registerComponent(const uint32_t hashId, size_t elementSize, size_t capacity);
    /**
     * Resizes the storage for a specific component type.
     * 
     * @param hashId Unique identifier for the component type.
     * @param newCapacity The new capacity for the storage.
     */
    void resizeComponent(const uint32_t hashId, size_t newCapacity);

    /**
     * Defers the attachment of a component to an entity.
     * 
     * @param e The entity to attach the component to.
     * @param hashId Unique identifier for the component type.
     * @param data Pointer to the component data.
     * @param size Size of the component data.
     */
    void attachComponentDeferred(Entity e, const uint32_t hashId, void* data, size_t size);
    /**
     * Defers the removal of a component from an entity.
     * 
     * @param e The entity to remove the component from.
     * @param hashId Unique identifier for the component type.
     */
    void removeComponentDeferred(Entity e, const uint32_t hashId);

    /**
     * Retrieves a pointer to the component instance for a given entity.
     * 
     * @param e The entity owning the component.
     * @param hashId Unique identifier for the component type.
     * @return Pointer to the component data, or nullptr if not found.
     */
    void* getComponent(Entity e, const uint32_t hashId);
    /**
     * Checks if an entity has a specific component.
     * 
     * @param e The entity to check.
     * @param hashId Unique identifier for the component type.
     * @return true if the entity has the component, false otherwise.
     */
    bool hasComponent(Entity e, const uint32_t hashId);
    /**
     * Retrieves the metadata for a specific component type.
     * 
     * @param hashId Unique identifier for the component type.
     * @return Pointer to the ComponentData structure.
     */
    ComponentData* getComponentData(const uint32_t hashId);

    /**
     * Callback triggered when a component is attached to an entity.
     * 
     * @param e The entity to which the component was attached.
     * @param hashId Unique identifier for the component type.
     */
    void onComponentAttached(Entity e, uint32_t hashId);
    /**
     * Callback triggered when a component is removed from an entity.
     * 
     * @param e The entity from which the component was removed.
     * @param hashId Unique identifier for the component type.
     */
    void onComponentRemoved(Entity e, const uint32_t hashId);

    /**
     * Registers a group of components for optimized querying.
     * 
     * @param componentHashIds Array of component identifiers defining the group.
     * @param count Number of components in the group.
     */
    void registerGroup(const uint32_t* componentHashIds, size_t count);


    /**
     * Checks if a value exists within an array.
     * 
     * @param array The array to search.
     * @param count Number of elements in the array.
     * @param value The value to search for.
     * @return true if the value is found, false otherwise.
     */
    bool contains(const uint32_t* array, size_t count, uint32_t value);
    /**
     * Checks if an entity has all components specified in the array.
     * 
     * @param e The entity to check.
     * @param comps Array of component identifiers.
     * @param count Number of components in the array.
     * @return true if the entity possesses all specified components, false otherwise.
     */
    bool hasAll(Entity e, const uint32_t* comps, size_t count);
    /**
     * Gets the number of entities that match a specific group of components.
     * 
     * @param compNames Array of component identifiers.
     * @param count Number of components in the array.
     * @return The number of matching entities.
     */
    size_t getGroupSize(const uint32_t* compNames, size_t count);

    /**
     * Gets the raw memory pointer for a component type.
     * 
     * @param hashId Unique identifier for the component type.
     * @param lock Whether to lock the component for access.
     * @return Pointer to the raw memory of the component storage.
     */
    void* getRawPtr(const uint32_t hashId, bool lock = false);
    /**
     * Gets the size of a single element for a given component type.
     * 
     * @param hashId Unique identifier for the component type.
     * @return The size of the component element in bytes.
     */
    size_t getComponentSize(const uint32_t hashId);

    /**
     * Gets the lock status for a specific component type.
     * 
     * @param hashId Unique identifier for the component type.
     * @return Reference to the atomic boolean lock.
     */
    std::atomic_bool& getComponentLock(const uint32_t hashId);
    /**
     * Sets the lock status for a specific component type.
     * 
     * @param hashId Unique identifier for the component type.
     * @param lock The lock state to set.
     */
    void setComponentLock(const uint32_t hashId, bool lock);

private:


    void attachComponent(Entity e, const uint32_t hashId, void* data);
    void removeComponent(Entity e, const uint32_t hashId);

    // component storage
    std::unordered_map<uint32_t, std::unique_ptr<ComponentData>> components;
    std::unordered_map<uint32_t, std::atomic_bool> componentLocks;
    std::vector<Group> groups;

    // command queues + synchronization
    std::atomic<ECSState> state{ECSState::Idle};
    std::vector<AttachCMD> attachQueue;
    std::vector<RemoveCMD> removeQueue;
    std::mutex cmdLock;
    std::unique_ptr<EngineArena> cmdMem = nullptr;
    std::vector<std::unique_ptr<EngineArena>> ECSDomainArenas; // one arena per component
    
};
