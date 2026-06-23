#pragma once
#include "engine/services/ECS/Entity/entity.h"
#include <vector>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>
#include "engine/services/MemoryAlloc/EngineArena.h"
#include "engine/services/MemoryAlloc/MemoryAlloc.h"

struct ComponentData {
    static size_t align_up(size_t size, size_t alignment) {
        return (size + alignment - 1) & ~(alignment - 1);
    }
    size_t groupedCount = 0;
    void* data = nullptr;
    size_t elementSize = 0;
    size_t capacity = 0;
    std::vector<uint32_t, ArenaAllocator<uint32_t>> dense;
    std::vector<uint32_t, ArenaAllocator<uint32_t>> sparse;
    EngineArena* arena_ptr;

    ComponentData(size_t elemSize, size_t cap, EngineArena* arena)
        : capacity(cap), 
        dense(ArenaAllocator<uint32_t>(arena)),
        sparse(ArenaAllocator<uint32_t>(arena)),
        arena_ptr(arena){
        elementSize = align_up(elemSize, 16);

        data = arena_ptr->allocate(elementSize * capacity, alignof(std::max_align_t));
        dense.reserve(cap);
        sparse.resize(cap, 0xFFFFFFFF);
    }

    void resize(size_t newCapacity) {
        if (newCapacity <= capacity) return;
        
        void* newData = arena_ptr->allocate(elementSize * newCapacity, alignof(std::max_align_t));
        
        for (size_t i = 0; i < dense.size(); ++i) {
            uint32_t entId = dense[i];
            std::memcpy(static_cast<char*>(newData) + elementSize * i, 
                        static_cast<char*>(data) + elementSize * i, 
                        elementSize);
        }
        
        data = newData;
        capacity = newCapacity;
        sparse.resize(capacity, 0xFFFFFFFF);
        
        for (size_t i = 0; i < dense.size(); ++i) {
            sparse[dense[i]] = static_cast<uint32_t>(i);
        }
    }

    ~ComponentData() = default;
    
    void* getRawPtr() { return data; }

    void attach(Entity e, void* elem) {
        if (e.id >= capacity) throw std::runtime_error("Entity ID exceeds capacity");
        if (sparse[e.id] != 0xFFFFFFFF) throw std::runtime_error("Entity already has component");

        sparse[e.id] = static_cast<uint32_t>(dense.size());
        dense.push_back(e.id);
        std::memcpy(static_cast<char*>(data) + elementSize * sparse[e.id], elem, elementSize);
    }
    void swapEntities(uint32_t idx1, uint32_t idx2){
        if (idx1 == idx2) return;

        uint32_t ent1 = dense[idx1];
        uint32_t ent2 = dense[idx2];
        dense[idx1] = ent2;
        dense[idx2] = ent1;
        sparse[ent1] = idx2;
        sparse[ent2] = idx1;
        
        char* ptr1 = static_cast<char*>(data) + (idx1 * elementSize);
        char* ptr2 = static_cast<char*>(data) + (idx2 * elementSize);
        
        // Use a stack buffer for small components, and a heap buffer for large ones
        char stackTemp[256];
        void* temp = nullptr;
        std::vector<char> heapTemp;
        
        if (elementSize <= 256) {
            temp = stackTemp;
        } else {
            heapTemp.resize(elementSize);
            temp = heapTemp.data();
        }
        
        std::memcpy(temp, ptr1, elementSize);
        std::memcpy(ptr1, ptr2, elementSize);
        std::memcpy(ptr2, temp, elementSize);
    }
    void moveToGroup(Entity e) {
        uint32_t currentIdx = sparse[e.id];
        if (currentIdx < groupedCount) return;

        swapEntities(currentIdx, groupedCount);
        groupedCount++;
    }

    void removeFromGroup(Entity e) {
        uint32_t currentIdx = sparse[e.id];
        if (currentIdx >= groupedCount) return;

        groupedCount--;
        swapEntities(currentIdx, groupedCount);
    }
    void* get(Entity e) {
        if (e.id >= capacity) throw std::runtime_error("Entity ID exceeds capacity");
        uint32_t idx = sparse[e.id];
        if (idx == 0xFFFFFFFF) throw std::runtime_error("Entity does not have component");
        return static_cast<char*>(data) + elementSize * idx;
    }

    void remove(Entity e) {
        if (e.id >= capacity) throw std::runtime_error("Entity ID exceeds capacity");
        uint32_t idx = sparse[e.id];
        if (idx == 0xFFFFFFFF) return;

        uint32_t lastEid = dense.back();
        std::memcpy(static_cast<char*>(data) + elementSize * idx,
                    static_cast<char*>(data) + elementSize * sparse[lastEid],
                    elementSize);
        dense[idx] = lastEid;
        sparse[lastEid] = idx;

        dense.pop_back();
        sparse[e.id] = 0xFFFFFFFF;
    }

    bool has(Entity e) {
        return e.id < capacity && sparse[e.id] != 0xFFFFFFFF;
    }
};
