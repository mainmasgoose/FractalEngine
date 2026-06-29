#pragma once
#include "ComponentManager.h"
#include "../Entity/entity.h"
#include "hash/hash.h"
#include "engine/FURCMD/FURCMD.h"
#include <cstdint>

struct attachComponentDeferredCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
    void* componentData;
    uint32_t dataSize;
};

struct removeComponentDeferredCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};

struct registerComponentCMDContext {
    uint32_t domainId;
    uint32_t componentId;
    uint32_t componentSize;
    uint32_t capacity;
};

struct flushCommandsCMDContext {
    uint32_t domainId;
};

struct getComponentCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};

struct hasComponentCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};

struct registerGroupCMDContext {
    uint32_t domainId;
    uint32_t* componentHashIds;
    uint32_t count;
};

struct containsCMDContext {
    uint32_t domainId;
    uint32_t* componentHashIds;
    uint32_t count;
    uint32_t value;
};

struct hasAllCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t* componentHashIds;
    uint32_t count;
};

struct getGroupSizeCMDContext {
    uint32_t domainId;
    uint32_t* componentHashIds;
    uint32_t count;
};

struct queryEntitiesCMDContext {
    uint32_t domainId;
    uint32_t* componentHashIds;
    uint32_t count;
};

struct getRawPtrCMDContext {
    uint32_t domainId;
    uint32_t componentId;
    bool lock;
};

struct onComponentAttachedCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};

struct onComponentRemovedCMDContext {
    uint32_t domainId;
    Entity entity;
    uint32_t componentId;
};
struct setComponentLockCMDContext {
    uint32_t domainId;
    uint32_t componentId;
    bool lock;
};
struct getComponentLockCMDContext {
    uint32_t domainId;
    uint32_t componentId;
};
struct resizeComponentCMDContext {
    uint32_t domainId;
    uint32_t componentId;
    uint32_t newCapacity;
};

class CMDomains {
public:
    CMDomains();
    ~CMDomains() = default;
    static void registerCMDomain(FURCMDPacket& packet);

    static void flushCommandsCMD(FURCMDPacket& packet);
    static void registerComponentCMD(FURCMDPacket& packet);
    static void resizeComponentCMD(FURCMDPacket& packet);

    static void attachComponentDeferredCMD(FURCMDPacket& packet);
    static void removeComponentDeferredCMD(FURCMDPacket& packet);

    static void getComponentCMD(FURCMDPacket& packet);
    static void hasComponentCMD(FURCMDPacket& packet);

    static void onComponentAttachedCMD(FURCMDPacket& packet);
    static void onComponentRemovedCMD(FURCMDPacket& packet);

    static void registerGroupCMD(FURCMDPacket& packet);

    static void containsCMD(FURCMDPacket& packet);
    static void hasAllCMD(FURCMDPacket& packet);
    static void getGroupSizeCMD(FURCMDPacket& packet);
    static void queryEntitiesCMD(FURCMDPacket& packet);
    static void getRawPtrCMD(FURCMDPacket& packet);

    static void getComponentLockCMD(FURCMDPacket& packet);
    static void setComponentLockCMD(FURCMDPacket& packet);
private:
    static ankerl::unordered_dense::map<uint32_t, ComponentManager*, IdentityHash> componentManagers;
    

};