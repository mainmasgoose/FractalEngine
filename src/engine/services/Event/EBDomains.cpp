#include "EBDomains.h"
#include "Engine/Engine.hpp"
#include "ankerl/unordered_dense.h"
#include "core/FractalKernel.h"
#include "hash/hash.h"
#include "services/Event/EventBus.h"
#include <cstdint>
#include <cstdio>
constexpr uint32_t registerEBHash = fnv1aHashConst("fractal_engine:event_bus:registerEBDomain");
constexpr uint32_t subscribeEventHash = fnv1aHashConst("fractal_engine:event_bus:subscribeEvent");
constexpr uint32_t emitEventHash = fnv1aHashConst("fractal_engine:event_bus:emitEvent");
constexpr uint32_t pushEventHash = fnv1aHashConst("fractal_engine:event_bus:pushEvent");
constexpr uint32_t processEventsHash = fnv1aHashConst("fractal_engine:event_bus:processEvents");
constexpr uint32_t resetEventsHash = fnv1aHashConst("fractal_engine:event_bus:resetEvents");
constexpr uint32_t unsubscribeEventHash = fnv1aHashConst("fractal_engine:event_bus:unsubscribeEvent");
ankerl::unordered_dense::map<uint32_t, EventBus*, IdentityHash> EBDomains::eventBuses;
EBDomains::EBDomains() {
    FractalKernel::instance().registerCMDMethod(registerEBHash, &registerEBDomain);
    FractalKernel::instance().registerCMDMethod(subscribeEventHash, &subscribeEventCMD);
    FractalKernel::instance().registerCMDMethod(emitEventHash, &emitEventCMD);
    FractalKernel::instance().registerCMDMethod(pushEventHash, &pushEventCMD);
    FractalKernel::instance().registerCMDMethod(processEventsHash, &processEventsCMD);
    FractalKernel::instance().registerCMDMethod(resetEventsHash, &resetEventsCMD);
    FractalKernel::instance().registerCMDMethod(unsubscribeEventHash, &unsubscribeEventCMD);
}
void EBDomains::registerEBDomain(FURCMDPacket &packet) {
    auto* ctx = reinterpret_cast<uint32_t*>(packet.payload);
    uint32_t domainId = *ctx;
    if (eventBuses.find(domainId) != eventBuses.end()) {
        std::fprintf(stderr, "Domain %u already exists!\n", domainId);
        return;
    }
    eventBuses[domainId] = new EventBus();
}
void EBDomains::emitEventCMD(FURCMDPacket &packet){
    emitEventCMDContext& context = *reinterpret_cast<emitEventCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = eventBuses.find(domainId);
    if (it != eventBuses.end()){
        it->second->emitEvent(context.eventId, context.data);
    }
}
void EBDomains::subscribeEventCMD(FURCMDPacket &packet){
    subscribeEventCMDContext& context = *reinterpret_cast<subscribeEventCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = eventBuses.find(domainId);
    if (it != eventBuses.end()){
        it->second->subscribe(context.eventId, context.cb, context.user, context.subscriberId);
    }
}
void EBDomains::pushEventCMD(FURCMDPacket &packet){
    pushEventCMDContext& context = *reinterpret_cast<pushEventCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = eventBuses.find(domainId);
    if (it != eventBuses.end()){
        it->second->pushEvent(context.eventId, context.data);
    }
}
void EBDomains::processEventsCMD(FURCMDPacket &packet){
    processEventsCMDContext& context = *reinterpret_cast<processEventsCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = eventBuses.find(domainId);
    if (it != eventBuses.end()){
        it->second->processEvents();
    }
}
void EBDomains::resetEventsCMD(FURCMDPacket &packet){
    resetEventsCMDContext& context = *reinterpret_cast<resetEventsCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = eventBuses.find(domainId);
    if (it != eventBuses.end()){
        it->second->reset();
    }
}
void EBDomains::unsubscribeEventCMD(FURCMDPacket &packet){
    unsubscribeEventCMDContext& context = *reinterpret_cast<unsubscribeEventCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;
    auto it = eventBuses.find(domainId);
    if (it != eventBuses.end()){
        it->second->unsubscribe(context.subscriberId);
    }
}
