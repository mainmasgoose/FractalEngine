#pragma once
#include "hash/hash.h"
#include <stdint.h>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "ankerl/unordered_dense.h"

// FUR CMD PACKET - FAST UTILITY ROUTER COMMAND PACKET
struct alignas(8) FURCMDPacket {
    uint32_t methodHash;    
    uint16_t payloadSize;   
    uint16_t flags;         
    void* payload;          
    void* outputBuffer;     
    uint64_t* fence = nullptr;        
};

typedef void (*FURMethod) (FURCMDPacket& packet);

struct InternalPacket {
    FURCMDPacket packet;
    std::vector<uint8_t> payloadData;
};

class FURCommandManager {
public:
    FURCommandManager();
    ~FURCommandManager();

    void registerFURMethod(uint32_t hashId, FURMethod method);
    void invoke(FURCMDPacket& packet);

private:
    void workerThread();

    ankerl::unordered_dense::map<uint32_t, FURMethod, IdentityHash> methodsTable;
    std::queue<InternalPacket> commandQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::thread worker;
    std::atomic<bool> running;
    
    // For tracing
    bool tracingEnabled = false;
};

