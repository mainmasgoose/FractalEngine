#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>
#include "engine/services/ECS/Entity/entity.h"
#include "engine/services/ECS/Component/ComponentManager.h"
#include "engine/services/MemoryAlloc/EngineArena.h"

struct Position {
    float x, y, z;
};

struct Velocity {
    float vx, vy, vz;
};

void test_entity_versioning() {
    std::cout << "Testing Entity Versioning..." << std::endl;
    EntityManager em;
    
    Entity e1 = em.createEntity();
    uint32_t id1 = e1.id;
    uint32_t ver1 = e1.version;
    
    assert(em.isValid(e1));
    
    em.destroyEntity(e1);
    assert(!em.isValid(e1));
    
    // Create a new entity, it should reuse the ID but have a different version
    Entity e2 = em.createEntity();
    assert(e2.id == id1);
    assert(e2.version != ver1);
    assert(em.isValid(e2));
    
    // The old handle should still be invalid
    assert(!em.isValid(e1));
    
    std::cout << "Entity Versioning: PASSED" << std::endl;
}

void test_component_lifecycle() {
    std::cout << "Testing Component Lifecycle..." << std::endl;
    EntityManager em;
    ComponentManager cm;
    
    uint32_t posHash = 0x12345678;
    cm.registerComponent(posHash, sizeof(Position), 100);
    
    Entity e1 = em.createEntity();
    Position p1{1.0f, 2.0f, 3.0f};
    
    cm.attachComponentDeferred(e1, posHash, &p1, sizeof(Position));
    cm.flushCommands();
    
    assert(cm.hasComponent(e1, posHash));
    Position* res = static_cast<Position*>(cm.getComponent(e1, posHash));
    assert(res->x == 1.0f && res->y == 2.0f && res->z == 3.0f);
    
    cm.removeComponentDeferred(e1, posHash);
    cm.flushCommands();
    
    assert(!cm.hasComponent(e1, posHash));
    
    std::cout << "Component Lifecycle: PASSED" << std::endl;
}

void test_paged_sparse_sets() {
    std::cout << "Testing Paged Sparse Sets..." << std::endl;
    EntityManager em;
    ComponentManager cm;
    
    uint32_t posHash = 0x12345678;
    // Small initial capacity to force paged growth if we create many entities
    cm.registerComponent(posHash, sizeof(Position), 10);
    
    std::vector<Entity> entities;
    for(int i = 0; i < 2000; ++i) {
        Entity e = em.createEntity();
        Position p{(float)i, 0, 0};
        cm.attachComponentDeferred(e, posHash, &p, sizeof(Position));
        entities.push_back(e);
    }
    cm.flushCommands();
    
    for(int i = 0; i < 2000; ++i) {
        Position* p = static_cast<Position*>(cm.getComponent(entities[i], posHash));
        assert(p->x == (float)i);
    }
    
    std::cout << "Paged Sparse Sets: PASSED" << std::endl;
}

void test_grouping_and_swapping() {
    std::cout << "Testing Grouping and Swapping..." << std::endl;
    EntityManager em;
    ComponentManager cm;
    
    uint32_t posHash = 0x1111;
    uint32_t velHash = 0x2222;
    
    cm.registerComponent(posHash, sizeof(Position), 100);
    cm.registerComponent(velHash, sizeof(Velocity), 100);
    
    uint32_t groupComps[] = {posHash, velHash};
    cm.registerGroup(groupComps, 2);
    
    Entity e1 = em.createEntity();
    Position p1{1, 1, 1};
    Velocity v1{1, 1, 1};
    cm.attachComponentDeferred(e1, posHash, &p1, sizeof(Position));
    cm.attachComponentDeferred(e1, velHash, &v1, sizeof(Velocity));
    
    Entity e2 = em.createEntity();
    Position p2{2, 2, 2};
    Velocity v2{2, 2, 2};
    cm.attachComponentDeferred(e2, posHash, &p2, sizeof(Position));
    cm.attachComponentDeferred(e2, velHash, &v2, sizeof(Velocity));
    
    cm.flushCommands();
    
    assert(cm.getGroupSize(groupComps, 2) == 2);
    
    // Remove component from e1, it should leave the group
    cm.removeComponentDeferred(e1, posHash);
    cm.flushCommands();
    
    assert(cm.getGroupSize(groupComps, 2) == 1);
    
    std::cout << "Grouping and Swapping: PASSED" << std::endl;
}

void test_ecs_queries() {
    std::cout << "Testing ECS Queries..." << std::endl;
    EntityManager em;
    ComponentManager cm;
    
    uint32_t posHash = 0x1111;
    uint32_t velHash = 0x2222;
    uint32_t accHash = 0x3333;
    
    cm.registerComponent(posHash, sizeof(Position), 100);
    cm.registerComponent(velHash, sizeof(Velocity), 100);
    cm.registerComponent(accHash, sizeof(Position), 100);
    
    uint32_t groupComps[] = {posHash, velHash};
    cm.registerGroup(groupComps, 2);
    
    Entity e1 = em.createEntity();
    Position p1{1,1,1}; Velocity v1{1,1,1};
    cm.attachComponentDeferred(e1, posHash, &p1, sizeof(Position));
    cm.attachComponentDeferred(e1, velHash, &v1, sizeof(Velocity));
    
    Entity e2 = em.createEntity();
    Position p2{2,2,2}; Velocity v2{2,2,2};
    cm.attachComponentDeferred(e2, posHash, &p2, sizeof(Position));
    cm.attachComponentDeferred(e2, velHash, &v2, sizeof(Velocity));
    
    Entity e3 = em.createEntity();
    Position p3{3,3,3};
    cm.attachComponentDeferred(e3, posHash, &p3, sizeof(Position));
    
    Entity e4 = em.createEntity();
    Velocity v4{4,4,4};
    cm.attachComponentDeferred(e4, velHash, &v4, sizeof(Velocity));
    
    Entity e5 = em.createEntity();
    Position p5{5,5,5}; Velocity v5{5,5,5}; Position a5{5,5,5};
    cm.attachComponentDeferred(e5, posHash, &p5, sizeof(Position));
    cm.attachComponentDeferred(e5, velHash, &v5, sizeof(Velocity));
    cm.attachComponentDeferred(e5, accHash, &a5, sizeof(Position));
    
    cm.flushCommands();
    
    uint32_t count = cm.getGroupSize(groupComps, 2);
    assert(count == 3);
    
    std::vector<Entity> results(count);
    cm.queryEntities(groupComps, 2, results.data());
    
    bool foundE1 = false, foundE2 = false, foundE5 = false;
    for(const auto& e : results) {
        if(e.id == e1.id) foundE1 = true;
        if(e.id == e2.id) foundE2 = true;
        if(e.id == e5.id) foundE5 = true;
    }
    assert(foundE1 && foundE2 && foundE5);
    
    uint32_t emptyGroup[] = {posHash, accHash};
    cm.registerGroup(emptyGroup, 2);
    uint32_t emptyCount = cm.getGroupSize(emptyGroup, 2);
    assert(emptyCount == 0);
    std::vector<Entity> emptyResults;
    cm.queryEntities(emptyGroup, 2, emptyResults.data());
    
    std::cout << "ECS Queries: PASSED" << std::endl;
}

void test_performance() {
    std::cout << "\n--- ECS Performance Test ---" << std::endl;
    EntityManager em;
    ComponentManager cm;
    
    const int ENTITY_COUNT = 100000;
    uint32_t posHash = 0x1111;
    uint32_t velHash = 0x2222;
    
    cm.registerComponent(posHash, sizeof(Position), 1024);
    cm.registerComponent(velHash, sizeof(Velocity), 1024);
    
    std::vector<Entity> entities;
    entities.reserve(ENTITY_COUNT);
    
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < ENTITY_COUNT; ++i) {
        entities.push_back(em.createEntity());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Create " << ENTITY_COUNT << " entities: " << elapsed.count() << " ms" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < ENTITY_COUNT; ++i) {
        Position p{(float)i, 0, 0};
        cm.attachComponentDeferred(entities[i], posHash, &p, sizeof(Position));
    }
    cm.flushCommands();
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Attach " << ENTITY_COUNT << " components: " << elapsed.count() << " ms" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    float sum = 0;
    for(int i = 0; i < ENTITY_COUNT; ++i) {
        Position* p = static_cast<Position*>(cm.getComponent(entities[i], posHash));
        sum += p->x;
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Iterate & access " << ENTITY_COUNT << " components: " << elapsed.count() << " ms (sum=" << sum << ")" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < ENTITY_COUNT; ++i) {
        cm.removeComponentDeferred(entities[i], posHash);
    }
    cm.flushCommands();
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Remove " << ENTITY_COUNT << " components: " << elapsed.count() << " ms" << std::endl;
    
    std::cout << "----------------------------\n" << std::endl;
}

int main() {
    try {
        test_entity_versioning();
        test_component_lifecycle();
    test_paged_sparse_sets();
    test_grouping_and_swapping();
    test_ecs_queries();
    test_performance();

        std::cout << "\nALL ECS TESTS PASSED!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
