#include "ModuleLoader.h"
#include "services/ModuleLoader/Module.h"
#include <iostream>
#include <cstring>

constexpr uint32_t loadModuleHash = fnv1aHashConst("fractal_engine:module_loader:loadModule");
constexpr uint32_t isLoadedHash = fnv1aHashConst("fractal_engine:module_loader:isLoaded");
constexpr uint32_t setModuleStateHash = fnv1aHashConst("fractal_engine:module_loader:setState");
constexpr uint32_t getModuleStateHash = fnv1aHashConst("fractal_engine:module_loader:getState");

ankerl::unordered_dense::map<uint32_t, std::unique_ptr<Module>, IdentityHash> ModuleLoader::modules;
std::mutex ModuleLoader::modulesMutex;

ModuleLoader::ModuleLoader() {
    FractalKernel::instance().registerCMDMethod(loadModuleHash, &loadModule);
    FractalKernel::instance().registerCMDMethod(isLoadedHash, &isLoadedCMD);
    FractalKernel::instance().registerCMDMethod(setModuleStateHash, &setModuleStateCMD);
    FractalKernel::instance().registerCMDMethod(getModuleStateHash, &getModuleStateCMD);
}

void ModuleLoader::loadModule(FURCMDPacket& packet) {
    auto context = static_cast<loadModuleContext*>(packet.payload);
    if (!context || context->path[0] == '\0') return;
    
    std::string pathCopy(context->path);
    uint32_t moduleHash = fnv1aHash(pathCopy.c_str());
    
    std::lock_guard<std::mutex> lock(modulesMutex);
    if (modules.contains(moduleHash)) {
        std::cout << "[ModuleLoader] Module already loaded: " << pathCopy << std::endl;
        return;
    }

    auto module = std::make_unique<Module>(pathCopy.c_str(), &FractalKernel::instance(), context->config);
    modules[moduleHash] = std::move(module);
    std::cout << "[ModuleLoader] Successfully loaded via VTable: " << pathCopy << std::endl;
}

void ModuleLoader::isLoadedCMD(FURCMDPacket& packet) {
    auto context = static_cast<isLoadedCMDContext*>(packet.payload);
    if (!context || !packet.outputBuffer) return;
    
    uint32_t moduleHash = fnv1aHash(context->path);
    
    std::lock_guard<std::mutex> lock(modulesMutex);
    bool loaded = modules.contains(moduleHash);
    
    std::memcpy(packet.outputBuffer, &loaded, sizeof(bool));
}

void ModuleLoader::setModuleStateCMD(FURCMDPacket& packet) {
    auto context = static_cast<setModuleStateContext*>(packet.payload);
    if (!context || context->path[0] == '\0') return;
    
    uint32_t moduleHash = fnv1aHash(context->path);
    
    std::lock_guard<std::mutex> lock(modulesMutex);
    auto it = modules.find(moduleHash);
    if (it != modules.end()) {
        it->second->setState(context->state);
    } else {
        std::cerr << "[ModuleLoader] Error: Cannot set state for unloaded module: " << context->path << std::endl;
    }
}

void ModuleLoader::getModuleStateCMD(FURCMDPacket& packet) {
    auto context = static_cast<getModuleStateContext*>(packet.payload);
    if (!context || context->path[0] == '\0') return;
    
    uint32_t moduleHash = fnv1aHash(context->path);
    
    std::lock_guard<std::mutex> lock(modulesMutex);
    auto it = modules.find(moduleHash);
    if (it != modules.end()) {
        if (packet.outputBuffer) {
            int state = it->second->getState();
            std::memcpy(packet.outputBuffer, &state, sizeof(int));
        }
    } else {
        if (packet.outputBuffer) {
            int notFound = -1;
            std::memcpy(packet.outputBuffer, &notFound, sizeof(int));
        }
    }
}