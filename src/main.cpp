#include "Engine/Engine.hpp"
#include "FURCMD/FURCMD.h"
#include "engine/core/FractalKernel.h" 
#include "hash/hash.h"
#include "services/Event/EBDomains.h"
#include "services/workScheduler/WSDomains.h"
#include "engine/services/ECS/Component/CMDomains.h"
#include "engine/services/SmartScheduler/SSDomains.h"
#include "engine/services/SystemExecution/SEDomains.h"
#include "services/ModuleLoader/ModuleLoader.h"
#include "services/ContainerLoader/ContainerLoader.h"
#include "engine/services/vfs/VFSDomains.h"
#include <cstdint>
#include <chrono>
#include <iostream>
#include <vector>
#include <atomic>
#include <filesystem>


struct Position { float x, y, z; };
struct Velocity { float vx, vy, vz; };

struct TaskContext {
    Position* pos;
    Velocity* vel;
    uint32_t count;
    std::atomic<uint32_t>* finished;
};

void processGroupTask(void* context) {
    auto ctx = reinterpret_cast<TaskContext*>(context);
    for (uint32_t i = 0; i < ctx->count; ++i) {
        ctx->pos[i].x += ctx->vel[i].vx;
        ctx->pos[i].y += ctx->vel[i].vy;
    }
    ctx->finished->fetch_add(1);
}

int main() {
    FractalKernel::instance().init();
    fe::Engine engine;
    engine.init();

    // Services initialization
    ComputeScheduler scheduler(4);
    CMDomains cmd;
    WSDomains wsd;
    EBDomains ebd;
    VFSDomains vfsd;
    SSDomains::init();
    SEDomains::init();


    ModuleLoader moduleLoader;
    ContainerLoader containerLoader;
    uint32_t domainId = 0;

    FURCMDPacket regCM{ fnv1aHashConst("fractal_engine:ecs:registerCMDomain"), sizeof(domainId), 0, &domainId };
    FractalKernel::instance().sendCMDPacket(regCM);

    bool running = true;


    while (running) {
        std::string cmdStr;
        if (!(std::cin >> cmdStr) || cmdStr == "exit") {
            running = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    SmartScheduler::instance().stop();
    return 0;
}
