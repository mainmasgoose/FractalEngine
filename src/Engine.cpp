#include "Engine/Engine.hpp"
#include <iostream>
#include "FURCMD/FURCMD.h"
#include "engine/core/FractalKernel.h"
#include "hash/hash.h"
#include "services/workScheduler/WSDomains.h"



namespace fe {

void Engine::run() {
    std::cout << "FractalEngine running (C++23)\n";

}
void Engine::init() {
    std::cout << "FractalEngine initializing...\n";
    FractalKernel::instance().init();
}
 
}