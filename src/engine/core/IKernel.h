#pragma once
#include <cstdint>

struct FURCMDPacket;
struct ModuleConfig;
class IKernel;

typedef void (*FURMethod)(FURCMDPacket& packet);

struct ModuleConfig {
    char containerRoot[1024];
    char userRoot[1024];
};

typedef void (*ModuleEntry)(IKernel* kernel, ModuleConfig config);

class IKernel {
public:
    virtual ~IKernel() = default;

    virtual void sendCMDPacket(FURCMDPacket& packet) = 0;
    virtual void registerCMDMethod(uint32_t hashId, FURMethod method) = 0;
};