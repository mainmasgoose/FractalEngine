#pragma once
#include "FractalSDK.h"
#include "IKernel.h"
#include "hash/hash.h"
#include <string>
#include <thread>
#include <filesystem>

/**
 * VFS - Virtual File System SDK.
 * Provides a high-level interface for resolving virtual paths
 * into absolute system paths via the Fractal Engine kernel.
 */
class VFS {
public:
    /**
     * Resolves a virtual path (e.g., "mod://AmethystY/assets/mesh.obj")
     * to an absolute system path.
     * Returns an empty string if resolution fails or SDK is not initialized.
     */
    static std::string ResolvePath(const char* virtualPath) {
        if (!virtualPath) return "";
        
        auto* sdk = FractalSDK::SDK::Get();
        if (!sdk) return ""; 
        
        char resolved[1024];
        resolved[0] = '\0';
        
        vfsResolveCMDContext ctx{virtualPath};
        FURCMDPacket packet;
        packet.methodHash = vfsResolveHash;
        packet.payload = &ctx;
        packet.payloadSize = sizeof(vfsResolveCMDContext);
        packet.outputBuffer = resolved;
        
        Ticket* ticket = sdk->allocateTicket();
        packet.fence = reinterpret_cast<uint64_t*>(&ticket->fence);
        
        sdk->sendPacket(packet);
        
        while (!ticket->isReady()) {
            std::this_thread::yield();
        }
        
        return std::string(resolved);
    }

    static bool FileExists(const char* virtualPath) {
        std::string path = ResolvePath(virtualPath);
        if (path.empty()) return false;
        return std::filesystem::exists(path);
    }

    static void SetContainerRoot(const char* path) {
        if (!path) return;
        auto* sdk = FractalSDK::SDK::Get();
        if (!sdk) return;

        vfsSetRootCMDContext ctx{path};
        FURCMDPacket packet;
        packet.methodHash = vfsSetContainerRootHash;
        packet.payloadSize = sizeof(ctx);
        packet.payload = &ctx;

        sdk->sendPacket(packet);
    }

    static void SetUserRoot(const char* path) {
        if (!path) return;
        auto* sdk = FractalSDK::SDK::Get();
        if (!sdk) return;

        vfsSetRootCMDContext ctx{path};
        FURCMDPacket packet;
        packet.methodHash = vfsSetUserRootHash;
        packet.payloadSize = sizeof(ctx);
        packet.payload = &ctx;

        sdk->sendPacket(packet);
    }
};
