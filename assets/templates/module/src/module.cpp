#include "headers/FractalSDK.h"
#include "headers/hash/hash.h"
#include "headers/IKernel.h"
#include "headers/ECS.h"
#include <iostream>

#ifdef _WIN32
    #define FRACTAL_EXPORT extern "C" __declspec(dllexport)
#else
    #define FRACTAL_EXPORT extern "C" __attribute__((visibility("default")))
#endif

FRACTAL_EXPORT void ModuleMain(IKernel* kernel) {
    FractalSDK::SDK::Initialize(kernel);
    
    std::cout << "[Module] Initialized successfully." << std::endl;
    
    // TODO: Implement module logic here
}
