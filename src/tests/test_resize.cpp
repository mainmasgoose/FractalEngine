#include "engine/services/ECS/Component/ComponentManager.h"
#include "engine/services/ECS/Entity/entity.h"
#include <iostream>
#include <cassert>


struct Position {
    float x, y, z;
};

int main() {
    ComponentManager manager;
    uint32_t posHash = 12345; // Simulated hash
    
    // Register component with small capacity
    manager.registerComponent(posHash, sizeof(Position), 2);
    
    // Attach 2 components
    Position p1 = {1.0f, 2.0f, 3.0f};
    Position p2 = {4.0f, 5.0f, 6.0f};
    Entity e1 = {0};
    Entity e2 = {1};
    
    manager.attachComponentDeferred(e1, posHash, &p1, sizeof(Position));
    manager.attachComponentDeferred(e2, posHash, &p2, sizeof(Position));
    manager.flushCommands();
    
    // Verify
    Position* gotP1 = (Position*)manager.getComponent(e1, posHash);
    assert(gotP1->x == 1.0f);
    
    // Resize
    manager.resizeComponent(posHash, 5);
    
    // Verify data integrity after resize
    Position* gotP1After = (Position*)manager.getComponent(e1, posHash);
    Position* gotP2After = (Position*)manager.getComponent(e2, posHash);
    
    assert(gotP1After->x == 1.0f);
    assert(gotP2After->x == 4.0f);
    
    // Attach more
    Position p3 = {7.0f, 8.0f, 9.0f};
    Entity e3 = {2};
    manager.attachComponentDeferred(e3, posHash, &p3, sizeof(Position));
    manager.flushCommands();
    
    Position* gotP3 = (Position*)manager.getComponent(e3, posHash);
    assert(gotP3->x == 7.0f);
    
    std::cout << "Resize test passed!" << std::endl;
    return 0;
}
