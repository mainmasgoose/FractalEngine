#pragma once
#include "VFS.hpp"
#include "FURCMD/FURCMD.h"
#include <cstdint>
#include <string>

struct vfsResolveCMDContext {
    const char* virtualPath;
};

struct vfsSetRootCMDContext {
    char path[512];
};

class VFSDomains {
public:
    VFSDomains();
    ~VFSDomains() = default;
    
    static void resolvePathCMD(FURCMDPacket& packet);
    static void setContainerRootCMD(FURCMDPacket& packet);
    static void getContainerRootCMD(FURCMDPacket& packet);
    static void setUserRootCMD(FURCMDPacket& packet);
    };