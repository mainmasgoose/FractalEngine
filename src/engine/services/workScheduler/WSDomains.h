#pragma once
#include "FURCMD/FURCMD.h"
#include "hash/hash.h"
#include "workScheduler.h"
struct scheduleTaskCMDContext {
    uint32_t domainId;
    ComputeTask task;
};
class WSDomains {
public:
    WSDomains();
    ~WSDomains() = default;
    static void registerWSDomain(FURCMDPacket& packet);
    static void scheduleTaskCMD(FURCMDPacket& packet);
private:
    static ankerl::unordered_dense::map<uint32_t, ComputeScheduler*, IdentityHash> computeSchedulers;
};