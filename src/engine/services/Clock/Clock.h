#pragma once
#include <chrono>

/**
 * Clock (Clock) - Core Component.
 * Responsible for tracking time and calculating delta time between frames.
 */
class Clock {
    public:
    /**
     * Initializes the clock.
     */
    Clock();
    ~Clock() = default;
    /**
     * Resets the clock's start and last time points.
     */
    void resetClock();
    /**
     * Updates the clock's time and calculates the delta time.
     */
    void update();
    /**
     * Gets the time elapsed since the last update.
     * 
     * @return The delta time in seconds.
     */
    float getDeltaTime() const;
    /**
     * Gets the total time elapsed since the clock started.
     * 
     * @return The total time in seconds.
     */
    float getTotalTime() const;
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point last_time;
        float deltaTime = 0.0f;
        float totalTime = 0.0f;

};
