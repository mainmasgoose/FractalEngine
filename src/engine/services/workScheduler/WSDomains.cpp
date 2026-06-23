#include "WSDomains.h"
#include "core/FractalKernel.h"
constexpr uint32_t registerWSDomainHash = fnv1aHashConst("fractal_engine:work_scheduler:registerWSDomain");
constexpr uint32_t scheduleTaskHash = fnv1aHashConst("fractal_engine:work_scheduler:scheduleTask");
ankerl::unordered_dense::map<uint32_t, ComputeScheduler*, IdentityHash> WSDomains::computeSchedulers;

WSDomains::WSDomains(){
    FractalKernel::instance().registerCMDMethod(registerWSDomainHash, &registerWSDomain);
    FractalKernel::instance().registerCMDMethod(scheduleTaskHash, &scheduleTaskCMD);
}
void WSDomains::registerWSDomain(FURCMDPacket& packet){
    uint32_t domainId = *reinterpret_cast<uint32_t*>(packet.payload);
    if (computeSchedulers.find(domainId) == computeSchedulers.end()){
        computeSchedulers[domainId] = new ComputeScheduler();
    }
}
void WSDomains::scheduleTaskCMD(FURCMDPacket& packet){
    scheduleTaskCMDContext& context = *reinterpret_cast<scheduleTaskCMDContext*>(packet.payload);
    uint32_t domainId = context.domainId;

    if (computeSchedulers.find(domainId) != computeSchedulers.end()){
        ComputeTask task = context.task;
        computeSchedulers[domainId]->scheduleTask(task);
    }
}
