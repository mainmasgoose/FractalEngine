#pragma once
#ifndef MODULE_H
#define MODULE_H

#ifdef _WIN32
    #include <windows.h>
    using LibHandle = HMODULE;
#else
    #include <dlfcn.h>
    #include <iostream>
    using LibHandle = void*;
#endif

#include "core/IKernel.h"
#include <atomic>

typedef void (*ModuleEntry)(IKernel* kernel, ModuleConfig config);

class Module {
public:
    enum State {
        Loading = 0,
        Ready = 1,
        Error = 2
    };

    Module(const char* path, IKernel* kernel, ModuleConfig config) {
        m_state.store(Loading);
        if (!load(path)) {
            m_state.store(Error);
            #ifndef _WIN32
            std::cerr << "[Module] dlopen error: " << dlerror() << std::endl;
            #endif
            return;
        }

        ModuleEntry mainFunc = (ModuleEntry)getSymbol("ModuleMain");
        if (mainFunc) {
            mainFunc(kernel, config);
        } else {
            std::cerr << "[Module] Could not find ModuleMain in " << path << std::endl;
            m_state.store(Error);
            unload();
        }
    }

    ~Module() { unload(); }

    bool isLoaded() const { return handle != nullptr; }
    void setState(int s) { m_state.store(s); }
    int getState() const { return m_state.load(); }

    void* getSymbol(const char* name) const {
        if (!handle) return nullptr;
#ifdef _WIN32
        return (void*)GetProcAddress(handle, name);
#else
        return dlsym(handle, name);
#endif
    }

private:
    LibHandle handle = nullptr;
    std::atomic<int> m_state{Loading};

    bool load(const char* path) {
#ifdef _WIN32
        handle = LoadLibraryA(path);
#else
        handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
#endif
        return handle != nullptr;
    }

    void unload() {
        if (handle) {
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            handle = nullptr;
        }
    }
};

#endif
