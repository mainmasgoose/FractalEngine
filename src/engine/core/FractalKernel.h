#pragma once
#ifndef FRACTAL_KERNEL
#define FRACTAL_KERNEL

#include <cstdint>
#include <memory>
#include "FURCMD/FURCMD.h"
#include "IKernel.h"
#include <stdint.h>
// FUR CMD PACKET - FAST UTILITY ROUTER COMMAND PACKET
class FractalKernel : public IKernel {
public:
    static FractalKernel& instance();

    FractalKernel* self();

    void sendCMDPacket(FURCMDPacket& packet) override;
    void registerCMDMethod(uint32_t hashId, FURMethod method) override;

    void init();

private:
    std::unique_ptr<FURCommandManager> FURCMDManager;
};

#endif // !FRACTAL_KERNEL
