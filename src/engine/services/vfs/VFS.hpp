#pragma once
#include <cstddef>

/**
 * VFS (Virtual File System) - Core Component.
 * Responsible for resolving virtual paths (mod://, core://, user://) into absolute OS paths.
 * 
 * Design goals:
 * - Zero allocations in Core.
 * - Absolute ABI stability via const char* and buffer-passing.
 * - No dependency on std::string in Core.
 */
class VFS {
public:
    static constexpr size_t MAX_PATH_LENGTH = 1024;
 
    /**
     * Resolves a virtual path to an absolute system path.

     * @param outPath      Buffer where the absolute path will be written.
     * @param bufferSize   Size of the provided buffer.
     * @return true if resolved successfully, false otherwise.
     */
    static bool Resolve(const char* virtualPath, char* outPath, size_t bufferSize);

    /**
     * Sets the root path for the currently active container.
     * All "mod://" and "core://" paths are resolved relative to this root.
     */
    static void SetContainerRoot(const char* rootPath);

    static void GetContainerRoot(char* outBuffer, size_t bufferSize);

    /**
     * Sets the root path for user data (AppData, ~/.config, etc.).
     * All "user://" paths are resolved relative to this root.
     */
    static void SetUserRoot(const char* userPath);

private:
    static bool StartsWith(const char* str, const char* prefix);
    
    static void SafeAppend(char* dest, const char* src, size_t maxLen);

    static char s_containerRoot[MAX_PATH_LENGTH];
    static char s_userRoot[MAX_PATH_LENGTH];
};