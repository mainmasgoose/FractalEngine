#pragma once
#include "EventBus.h"
#include "FURCMD/FURCMD.h"
struct subscribeEventCMDContext {
    uint32_t domainId;
    uint32_t eventId;
    EventCallback cb;
    void* user;
    uint32_t subscriberId;
};
struct emitEventCMDContext {
    uint32_t domainId;
    uint32_t eventId;
    EventData data;
};
struct pushEventCMDContext {
    uint32_t domainId;
    uint32_t eventId;
    EventData data;
};
struct processEventsCMDContext {
    uint32_t domainId;
};
struct resetEventsCMDContext {
    uint32_t domainId;
};
struct unsubscribeEventCMDContext {
    uint32_t domainId;
    uint32_t subscriberId;
};


class EBDomains {
public:
    EBDomains();
    ~EBDomains() = default;
    static void registerEBDomain(FURCMDPacket& packet);
    static void subscribeEventCMD(FURCMDPacket& packet);
    static void emitEventCMD(FURCMDPacket& packet);
    static void pushEventCMD(FURCMDPacket& packet);
    static void processEventsCMD(FURCMDPacket& packet);
    static void resetEventsCMD(FURCMDPacket& packet);
    static void unsubscribeEventCMD(FURCMDPacket& packet);
private:
    static ankerl::unordered_dense::map<uint32_t, EventBus*, IdentityHash> eventBuses;

};