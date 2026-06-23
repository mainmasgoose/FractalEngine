#include "FractalKernel.h"
#include <stdexcept>

FractalKernel& FractalKernel::instance() {
    static FractalKernel instance;
    return instance;
}
void FractalKernel::sendCMDPacket(FURCMDPacket& packet) {
    if (FURCMDManager) {
        FURCMDManager->invoke(packet);
    } else {
        throw std::runtime_error("FURCMDManager not initialized");
    }
}

void FractalKernel::registerCMDMethod(uint32_t hashId, FURMethod method) {
    if (FURCMDManager) {
        FURCMDManager->registerFURMethod(hashId, method);
    } else {
        throw std::runtime_error("FURCMDManager not initialized");
    }
}

void FractalKernel::init() {
    if (!FURCMDManager) {
        FURCMDManager = std::make_unique<FURCommandManager>();
    }
}