#include "VFS.hpp"
#include <cstring>

char VFS::s_containerRoot[VFS::MAX_PATH_LENGTH] = {0};
char VFS::s_userRoot[VFS::MAX_PATH_LENGTH] = {0};

bool VFS::StartsWith(const char* str, const char* prefix) {
    if (!str || !prefix) return false;
    size_t prefixLen = std::strlen(prefix);
    return std::strncmp(str, prefix, prefixLen) == 0;
}

void VFS::SafeAppend(char* dest, const char* src, size_t maxLen) {
    if (!dest || !src) return;
    size_t currentLen = std::strlen(dest);
    if (currentLen >= maxLen) return;

    size_t srcLen = std::strlen(src);
    size_t spaceLeft = maxLen - currentLen - 1;
    size_t toCopy = (srcLen < spaceLeft) ? srcLen : spaceLeft;
    
    std::memcpy(dest + currentLen, src, toCopy);
    dest[currentLen + toCopy] = '\0';
}

bool VFS::Resolve(const char* virtualPath, char* outPath, size_t bufferSize) {
    if (!virtualPath || !outPath || bufferSize == 0) return false;
    
    outPath[0] = '\0';

    if (StartsWith(virtualPath, "mod://")) {
        const char* pathAfterPrefix = virtualPath + 6;
        
        std::strncpy(outPath, s_containerRoot, bufferSize - 1);
        size_t len = std::strlen(outPath);
        if (len > 0 && outPath[len-1] != '/') {
            if (len < bufferSize - 1) outPath[len] = '/';
        }
        
        // We remove the hardcoded "modules/" to allow the container root 
        // to define the base path for modules directly.
        SafeAppend(outPath, pathAfterPrefix, bufferSize);
    }
    else if (StartsWith(virtualPath, "core://")) {
        const char* pathAfterPrefix = virtualPath + 7;
        
        std::strncpy(outPath, s_containerRoot, bufferSize - 1);
        size_t len = std::strlen(outPath);
        if (len > 0 && outPath[len-1] != '/') {
            if (len < bufferSize - 1) outPath[len] = '/';
        }
        
        SafeAppend(outPath, "core/", bufferSize);
        SafeAppend(outPath, pathAfterPrefix, bufferSize);
    }
    else if (StartsWith(virtualPath, "user://")) {
        const char* pathAfterPrefix = virtualPath + 7;
        
        std::strncpy(outPath, s_userRoot, bufferSize - 1);
        size_t len = std::strlen(outPath);
        if (len > 0 && outPath[len-1] != '/') {
            if (len < bufferSize - 1) outPath[len] = '/';
        }
        
        SafeAppend(outPath, pathAfterPrefix, bufferSize);
    }
    else {
        return false;
    }
    
    return std::strlen(outPath) < bufferSize;
}

void VFS::SetContainerRoot(const char* rootPath) {
    if (!rootPath) return;
    std::strncpy(s_containerRoot, rootPath, MAX_PATH_LENGTH - 1);
    s_containerRoot[MAX_PATH_LENGTH - 1] = '\0';
}

void VFS::GetContainerRoot(char* outBuffer, size_t bufferSize) {
    if (!outBuffer || bufferSize == 0) return;
    std::strncpy(outBuffer, s_containerRoot, bufferSize - 1);
    outBuffer[bufferSize - 1] = '\0';
}

void VFS::SetUserRoot(const char* userPath) {
    if (!userPath) return;
    std::strncpy(s_userRoot, userPath, MAX_PATH_LENGTH - 1);
    s_userRoot[MAX_PATH_LENGTH - 1] = '\0';
}