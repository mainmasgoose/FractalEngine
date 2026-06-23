#include "workScheduler.h"
#include <cstddef>
#include <mutex>

ComputeScheduler* ComputeScheduler::instance = nullptr;

ComputeScheduler::ComputeScheduler(size_t threadsCount){
    if (threadsCount == 0) {
        threadsCount = 1;
    }
    instance = this;
    run = true;
    for (size_t i = 0; i < threadsCount ; i++ ){
        workers.emplace_back(&ComputeScheduler::workerThread, this);
    };

    
};
ComputeScheduler::~ComputeScheduler(){
    run = false;
    condition.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable()) worker.join();
    };
};
void ComputeScheduler::scheduleTask(ComputeTask task) {
    while (!taskQueue.push(task)) {
        std::this_thread::yield(); 
    }
        
    condition.notify_one();
}
