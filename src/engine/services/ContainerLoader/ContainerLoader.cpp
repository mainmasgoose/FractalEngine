#include "ContainerLoader.h"
#include "core/FractalKernel.h"
#include "hash/hash.h"
#include "services/vfs/VFSDomains.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include "json.hpp"

using json = nlohmann::json;

struct loadModuleContext {
    char path[512];
    ModuleConfig config;
};
struct getModuleStateContext {
    char path[512];
};

constexpr uint32_t reloadHash = fnv1aHashConst("fractal_engine:container_loader:reload");
constexpr uint32_t loadModuleHash = fnv1aHashConst("fractal_engine:module_loader:loadModule");
constexpr uint32_t getModuleStateHash = fnv1aHashConst("fractal_engine:module_loader:getState");

ContainerLoader::ContainerLoader() {
    FractalKernel::instance().registerCMDMethod(reloadHash, &reloadCMD);
    loadActiveContainer();
}

std::string ContainerLoader::getActiveContainerName() {
    std::ifstream file("active_container.json");
    if (!file.is_open()) return "";
    json j;
    file >> j;
    return j.value("active_container", "");
}

void ContainerLoader::loadActiveContainer() {
    std::string contName = getActiveContainerName();
    if (contName.empty()) {
        std::cout << "[ContainerLoader] No active container found in active_container.json\n";
        return;
    }

    std::string path = "container/" + contName + "/container.json";
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[ContainerLoader] Failed to open: " << path << "\n";
        return;
    }

    json j;
    file >> j;

    std::cout << "[ContainerLoader] Loading container: " << contName << " from " << path << "\n";
    
    std::string rootDir = "container/" + contName + "/";
    ModuleConfig config;
    std::strncpy(config.containerRoot, rootDir.c_str(), sizeof(config.containerRoot) - 1);
    std::strncpy(config.userRoot, "user_data/", sizeof(config.userRoot) - 1);

    for (const auto& modId : j["load_order"]) {
        std::string modPath = "container/" + contName + "/bin/" + modId.get<std::string>() + ".so";
        
        // 1. Request load
        loadModuleContext loadCtx;
        std::strncpy(loadCtx.path, modPath.c_str(), sizeof(loadCtx.path) - 1);
        loadCtx.config = config;
        std::strncpy(loadCtx.config.loadedModulePath, modPath.c_str(), sizeof(loadCtx.config.loadedModulePath) - 1);

        FURCMDPacket loadPacket;
        loadPacket.methodHash = loadModuleHash;
        loadPacket.payload = &loadCtx;
        loadPacket.payloadSize = sizeof(loadModuleContext);
        
        std::cout << "[ContainerLoader] Requesting load: " << modPath << "\n";
        FractalKernel::instance().sendCMDPacket(loadPacket);

        // 2. Wait for Ready state
        getModuleStateContext stateCtx;
        std::strncpy(stateCtx.path, modPath.c_str(), sizeof(stateCtx.path) - 1);
        stateCtx.path[sizeof(stateCtx.path) - 1] = '\0';

        FURCMDPacket statePacket;
        statePacket.methodHash = getModuleStateHash;
        statePacket.payload = &stateCtx;
        statePacket.payloadSize = sizeof(getModuleStateContext);
        statePacket.outputBuffer = new char[sizeof(int)]; 

        while (true) {
            FractalKernel::instance().sendCMDPacket(statePacket);
            
            int currentState = 0;
            std::memcpy(&currentState, statePacket.outputBuffer, sizeof(int));
            
            if (currentState == 1) { // Ready
                std::cout << "[ContainerLoader] Module " << modId << " is READY." << std::endl;
                break;
            } else if (currentState == 2) { // Error
                std::cerr << "[ContainerLoader] Module " << modId << " encountered an ERROR during init." << std::endl;
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        delete[] (char*)statePacket.outputBuffer;
    }
}

void ContainerLoader::reloadCMD(FURCMDPacket& packet) {
    std::cout << "[ContainerLoader] Reloading active container...\n";
}