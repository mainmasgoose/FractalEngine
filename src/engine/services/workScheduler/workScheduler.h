#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string.h>
#include "TaskQueue.h"
typedef void (*w_task_fn)(void* context);
struct ComputeTask {
    w_task_fn fn;
    void* context;
};
/**
 * ComputeScheduler (Compute Scheduler) - Core Component.
 * Manages a pool of worker threads to execute compute tasks asynchronously.
 */
class ComputeScheduler {
public:
    /**
     * Initializes the ComputeScheduler with a specified number of worker threads.
     * 
     * @param threadsCount Number of threads to spawn for the pool.
     * @return void
     */
    ComputeScheduler(size_t threadsCount = std::thread::hardware_concurrency()-1);

    /**
     * Destructor for ComputeScheduler. Stops all worker threads.
     * 
     * @return void
     */
    ~ComputeScheduler();

    /**
     * Schedules a compute task for asynchronous execution.
     * 
     * @param task The task to be scheduled.
     * @return void
     */
    void scheduleTask(ComputeTask task);

    /**
     * Retrieves the global instance of the ComputeScheduler.
     * 
     * @return Pointer to the ComputeScheduler instance.
     */
    static ComputeScheduler* get() { return instance; }
private:
    static ComputeScheduler* instance;

    /**
     * The main loop for worker threads, processing tasks from the queue.
     * 
     * @return void
     */
    void workerThread() {
        while (run.load(std::memory_order_relaxed)) {
            auto task = taskQueue.pop();
            if (task) {
                task->fn(task->context);
            } else {
                std::unique_lock<std::mutex> lock(waitMutex);
                condition.wait(lock, [this] { 
                    return !taskQueue.empty() || !run; 
                });
            }
        }
    }
    TaskQueue<ComputeTask> taskQueue;
    std::mutex waitMutex;
    std::condition_variable condition;
    std::atomic<bool> run;
    std::vector<std::thread> workers;
};
