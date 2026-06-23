#include "FURCMD.h"
#include <assert.h>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>

FURCommandManager::FURCommandManager() : running(true) {
    worker = std::thread(&FURCommandManager::workerThread, this);
}

FURCommandManager::~FURCommandManager() {
    running = false;
    cv.notify_one();
    if (worker.joinable()) {
        worker.join();
    }
}

void FURCommandManager::registerFURMethod(uint32_t hashId, FURMethod method) {
    std::lock_guard<std::mutex> lock(queueMutex);
    methodsTable.emplace(hashId, method);
}

void FURCommandManager::invoke(FURCMDPacket &packet) {
    InternalPacket internal;
    internal.packet = packet;
    if (packet.payload && packet.payloadSize > 0) {
        internal.payloadData.resize(packet.payloadSize);
        std::memcpy(internal.payloadData.data(), packet.payload,
                    packet.payloadSize);
        internal.packet.payload = internal.payloadData.data();
    }
 
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        commandQueue.push(std::move(internal));
    }
    cv.notify_one();
}

void FURCommandManager::workerThread() {
    while (running) {
        InternalPacket internal;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] { return !commandQueue.empty() || !running; });
            if (!running) break;
            internal = std::move(commandQueue.front());
            commandQueue.pop();
        }
        
        auto it = methodsTable.find(internal.packet.methodHash);
        if (it != methodsTable.end()) [[likely]] {
            it->second(internal.packet);
 
            if (internal.packet.fence != nullptr) {
                try {
                    auto activeFence = std::atomic_ref<int>(*reinterpret_cast<int*>(internal.packet.fence));
                    activeFence.store(1, std::memory_order_release);
                } catch (...) {
                }
            }
        } else {
            std::cerr << "[FURCMD Manager] UNKNOWN METHOD HASH: 0x" << std::hex << internal.packet.methodHash << std::dec 
                      << " | Payload Size: " << internal.packet.payloadSize << std::endl;
            
            if (internal.packet.fence != nullptr) {
                try {
                    auto activeFence = std::atomic_ref<int>(*reinterpret_cast<int*>(internal.packet.fence));
                    activeFence.store(1, std::memory_order_release);
                } catch (...) {
                }
            }
        }
    }
}