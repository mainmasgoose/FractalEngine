#include "CMDomains.h"
#include "../../MemoryAlloc/EngineArena.h"
#include "ComponentManager.h"
#include "hash/hash.h"
#include "FURCMD/FURCMD.h"
#include "core/FractalKernel.h"
#include <atomic>
#include <cstdint>
#include <cstring>
constexpr uint32_t registerCMDomainHash = fnv1aHashConst("fractal_engine:ecs:registerCMDomain");
constexpr uint32_t flushCommandsHash = fnv1aHashConst("fractal_engine:ecs:flushCommands");
constexpr uint32_t registerComponentHash = fnv1aHashConst("fractal_engine:ecs:registerComponent");
constexpr uint32_t attachComponentDeferredHash = fnv1aHashConst("fractal_engine:ecs:attachComponentDeferred");
constexpr uint32_t removeComponentDeferredHash = fnv1aHashConst("fractal_engine:ecs:removeComponentDeferred");
constexpr uint32_t getComponentHash = fnv1aHashConst("fractal_engine:ecs:getComponent");
constexpr uint32_t hasComponentHash = fnv1aHashConst("fractal_engine:ecs:hasComponent");
constexpr uint32_t onComponentAttachedHash = fnv1aHashConst("fractal_engine:ecs:onComponentAttached");
constexpr uint32_t onComponentRemovedHash = fnv1aHashConst("fractal_engine:ecs:onComponentRemoved");
constexpr uint32_t registerGroupHash = fnv1aHashConst("fractal_engine:ecs:registerGroup");
constexpr uint32_t containsHash = fnv1aHashConst("fractal_engine:ecs:contains");
constexpr uint32_t hasAllHash = fnv1aHashConst("fractal_engine:ecs:hasAll");
constexpr uint32_t getGroupSizeHash = fnv1aHashConst("fractal_engine:ecs:getGroupSize");
constexpr uint32_t getRawPtrHash = fnv1aHashConst("fractal_engine:ecs:getRawPtr");
constexpr uint32_t getComponentLockHash = fnv1aHashConst("fractal_engine:ecs:getComponentLock");
constexpr uint32_t setComponentLockHash = fnv1aHashConst("fractal_engine:ecs:setComponentLock");
constexpr uint32_t resizeComponentHash = fnv1aHashConst("fractal_engine:ecs:resizeComponent");

ankerl::unordered_dense::map<uint32_t, ComponentManager*, IdentityHash> CMDomains::componentManagers;

CMDomains::CMDomains(){
    FractalKernel::instance().registerCMDMethod(registerCMDomainHash, &registerCMDomain);
    FractalKernel::instance().registerCMDMethod(flushCommandsHash, &flushCommandsCMD);
    FractalKernel::instance().registerCMDMethod(registerComponentHash, &registerComponentCMD);
    FractalKernel::instance().registerCMDMethod(resizeComponentHash, &resizeComponentCMD);
    FractalKernel::instance().registerCMDMethod(attachComponentDeferredHash, &attachComponentDeferredCMD);
    FractalKernel::instance().registerCMDMethod(removeComponentDeferredHash, &removeComponentDeferredCMD);
    FractalKernel::instance().registerCMDMethod(getComponentHash, &getComponentCMD);
    FractalKernel::instance().registerCMDMethod(hasComponentHash, &hasComponentCMD);
    FractalKernel::instance().registerCMDMethod(onComponentAttachedHash, &onComponentAttachedCMD);
    FractalKernel::instance().registerCMDMethod(onComponentRemovedHash, &onComponentRemovedCMD);
    FractalKernel::instance().registerCMDMethod(registerGroupHash, &registerGroupCMD);
    FractalKernel::instance().registerCMDMethod(containsHash, &containsCMD);
    FractalKernel::instance().registerCMDMethod(hasAllHash, &hasAllCMD);
    FractalKernel::instance().registerCMDMethod(getGroupSizeHash, &getGroupSizeCMD);
    FractalKernel::instance().registerCMDMethod(getRawPtrHash, &getRawPtrCMD);
    FractalKernel::instance().registerCMDMethod(getComponentLockHash, &getComponentLockCMD);
    FractalKernel::instance().registerCMDMethod(setComponentLockHash, &setComponentLockCMD);
}

void CMDomains::resizeComponentCMD(FURCMDPacket& packet) {
    resizeComponentCMDContext& context = *reinterpret_cast<resizeComponentCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    if (componentManagers.find(domainId) != componentManagers.end()){
        componentManagers[domainId]->resizeComponent(context.componentId, context.newCapacity);
    }
}
void CMDomains::registerCMDomain(FURCMDPacket& packet) {
    uint32_t domainId = *reinterpret_cast<uint32_t*>(packet.payload);
    if (componentManagers.find(domainId) == componentManagers.end()){
        componentManagers[domainId] = new ComponentManager();
    }
}
void CMDomains::attachComponentDeferredCMD(FURCMDPacket &packet) {
    attachComponentDeferredCMDContext& context = *reinterpret_cast<attachComponentDeferredCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    if (componentManagers.find(domainId) != componentManagers.end()){
        componentManagers[domainId]->attachComponentDeferred(context.entity, context.componentId, context.componentData, context.dataSize);
    }
}
void CMDomains::removeComponentDeferredCMD(FURCMDPacket &packet){
    removeComponentDeferredCMDContext& context = *reinterpret_cast<removeComponentDeferredCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    if (componentManagers.find(domainId) != componentManagers.end()){
        componentManagers[domainId]->removeComponentDeferred(context.entity, context.componentId);
    }
}
void CMDomains::registerComponentCMD(FURCMDPacket &packet){
    registerComponentCMDContext& context = *reinterpret_cast<registerComponentCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    if (componentManagers.find(domainId) != componentManagers.end()){
        componentManagers[domainId]->registerComponent(context.componentId, context.componentSize, context.capacity);
    }
}
void CMDomains::flushCommandsCMD(FURCMDPacket &packet){
    flushCommandsCMDContext& context = *reinterpret_cast<flushCommandsCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    if (componentManagers.find(domainId) != componentManagers.end()){
        componentManagers[domainId]->flushCommands();
    }
}
void CMDomains::getComponentCMD(FURCMDPacket &packet) {
    getComponentCMDContext& context = *reinterpret_cast<getComponentCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = componentManagers.find(domainId);  
    if (it != componentManagers.end()) {
        auto* manager = it->second;
        void* compPtr = manager->getComponent(context.entity, context.componentId);
        if (packet.outputBuffer) {
            *reinterpret_cast<void**>(packet.outputBuffer) = compPtr;
        }
    }
}
void CMDomains::hasComponentCMD(FURCMDPacket &packet) {
    hasComponentCMDContext& context = *reinterpret_cast<hasComponentCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = componentManagers.find(domainId);
    if (it != componentManagers.end()) {
        auto* manager = it->second;
        bool hasComp = manager->hasComponent(context.entity, context.componentId);
        *reinterpret_cast<bool*>(packet.outputBuffer) = hasComp;
    }
}
void CMDomains::registerGroupCMD(FURCMDPacket &packet) {
    registerGroupCMDContext& context = *reinterpret_cast<registerGroupCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    if (componentManagers.find(domainId) != componentManagers.end()){
        componentManagers[domainId]->registerGroup(context.componentHashIds, context.count);
    }
}
void CMDomains::onComponentAttachedCMD(FURCMDPacket &packet) {
    onComponentAttachedCMDContext& context = *reinterpret_cast<onComponentAttachedCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    if (componentManagers.find(domainId) != componentManagers.end()){
        componentManagers[domainId]->onComponentAttached(context.entity, context.componentId);
    }
}
void CMDomains::onComponentRemovedCMD(FURCMDPacket &packet) {
    onComponentRemovedCMDContext& context = *reinterpret_cast<onComponentRemovedCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    if (componentManagers.find(domainId) != componentManagers.end()){
        componentManagers[domainId]->onComponentRemoved(context.entity, context.componentId);
    }
}
void CMDomains::containsCMD(FURCMDPacket &packet) {
    containsCMDContext& context = *reinterpret_cast<containsCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = componentManagers.find(domainId);
    if (it != componentManagers.end()) {
        auto* manager = it->second;
        bool result = manager->contains(context.componentHashIds, context.count, context.value);
        *reinterpret_cast<bool*>(packet.outputBuffer) = result;
    }
}
void CMDomains::hasAllCMD(FURCMDPacket &packet) {
    hasAllCMDContext& context = *reinterpret_cast<hasAllCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = componentManagers.find(domainId);
    if (it != componentManagers.end()) {
        auto* manager = it->second;
        bool result = manager->hasAll(context.entity, context.componentHashIds, context.count);
        *reinterpret_cast<bool*>(packet.outputBuffer) = result;
    }
}
void CMDomains::getGroupSizeCMD(FURCMDPacket &packet) {
    getGroupSizeCMDContext& context = *reinterpret_cast<getGroupSizeCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = componentManagers.find(domainId);
    if (it != componentManagers.end()) {
        auto* manager = it->second;
        size_t groupSize = manager->getGroupSize(context.componentHashIds, context.count);
        *reinterpret_cast<size_t*>(packet.outputBuffer) = groupSize;
    }
}
void CMDomains::getRawPtrCMD(FURCMDPacket &packet) {
    getRawPtrCMDContext& context = *reinterpret_cast<getRawPtrCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = componentManagers.find(domainId);
    if (it != componentManagers.end()) {
        auto* manager = it->second;
        void* rawPtr = manager->getRawPtr(context.componentId, context.lock);
        *reinterpret_cast<void**>(packet.outputBuffer) = rawPtr;
    }
}
void CMDomains::getComponentLockCMD(FURCMDPacket &packet) {
    getComponentLockCMDContext& context = *reinterpret_cast<getComponentLockCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = componentManagers.find(domainId);
    if (it != componentManagers.end()) {
        auto* manager = it->second;
        std::atomic_bool& lock = manager->getComponentLock(context.componentId);
        *reinterpret_cast<std::atomic_bool**>(packet.outputBuffer) = &lock;
    }
}
void CMDomains::setComponentLockCMD(FURCMDPacket &packet) {
    setComponentLockCMDContext& context = *reinterpret_cast<setComponentLockCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = componentManagers.find(domainId);
    if (it != componentManagers.end()) {
        auto* manager = it->second;
        manager->setComponentLock(context.componentId, context.lock);
    }
}
