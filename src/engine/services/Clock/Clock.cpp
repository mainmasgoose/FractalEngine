#include "Clock.h"
Clock::Clock() {
    start_time = std::chrono::high_resolution_clock::now();
    last_time = start_time;
}

void Clock::update() {
    auto now = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration<float>(now - last_time).count();
    last_time = now;
    totalTime = std::chrono::duration<float>(now - start_time).count();

}
void Clock::resetClock() {
    start_time = std::chrono::high_resolution_clock::now();
    last_time = start_time;
}

float Clock::getDeltaTime() const {
    return deltaTime;
}
float Clock::getTotalTime() const {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> total = now - start_time;
    return total.count();
}
