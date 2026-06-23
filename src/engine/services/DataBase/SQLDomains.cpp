#include "SQLDomains.h"
#include "core/FractalKernel.h"
#include "hash/hash.h"
#include "services/DataBase/SQLite.h"
#include <cstdint>

constexpr uint32_t registerSQLDomainHash = fnv1aHashConst("fractal_engine:sqldb:registerSQLDomain");
constexpr uint32_t openCMDHash = fnv1aHashConst("fractal_engine:sqldb:openCMD");
constexpr uint32_t executeCMDHash = fnv1aHashConst("fractal_engine:sqldb:executeCMD");
constexpr uint32_t setStringCMDHash = fnv1aHashConst("fractal_engine:sqldb:setStringCMD");
constexpr uint32_t getStringCMDHash = fnv1aHashConst("fractal_engine:sqldb:getStringCMD");
constexpr uint32_t existsCMDHash = fnv1aHashConst("fractal_engine:sqldb:existsCMD");
constexpr uint32_t closeCMDHash = fnv1aHashConst("fractal_engine:sqldb:closeCMD");
constexpr uint32_t isOpenCMDHash = fnv1aHashConst("fractal_engine:sqldb:isOpenCMD");

ankerl::unordered_dense::map<uint32_t, SQLiteDB*, IdentityHash> SQLDomains::sqlDomains;

SQLDomains::SQLDomains(){
    FractalKernel::instance().registerCMDMethod(registerSQLDomainHash, &registerSQLDomain);
    FractalKernel::instance().registerCMDMethod(openCMDHash, &openCMD);
    FractalKernel::instance().registerCMDMethod(executeCMDHash, &executeCMD);
    FractalKernel::instance().registerCMDMethod(setStringCMDHash, &setStringCMD);
    FractalKernel::instance().registerCMDMethod(getStringCMDHash, &getStringCMD);
    FractalKernel::instance().registerCMDMethod(existsCMDHash, &existsCMD);
    FractalKernel::instance().registerCMDMethod(closeCMDHash, &closeCMD);
    FractalKernel::instance().registerCMDMethod(isOpenCMDHash, &isOpenCMD);
}
void SQLDomains::registerSQLDomain(FURCMDPacket& packet){
    auto* context = reinterpret_cast<registerSQLDomainContext*>(packet.payload);
    if (sqlDomains.find(context->domainId) == sqlDomains.end()){
        sqlDomains[context->domainId] = new SQLiteDB();
    }
}
void SQLDomains::openCMD(FURCMDPacket& packet){
    auto* context = reinterpret_cast<openCMDContext*>(packet.payload);
    auto it = sqlDomains.find(context->domainId);
    if (it != sqlDomains.end()){
        bool result = it->second->open(context->dbPath);
        *reinterpret_cast<bool*>(packet.outputBuffer) = result;
    }
}
void SQLDomains::executeCMD(FURCMDPacket& packet){
    auto* context = reinterpret_cast<executeCMDContext*>(packet.payload);
    auto it = sqlDomains.find(context->domainId);
    if (it != sqlDomains.end()){
        bool result = it->second->execute(context->sql);
        *reinterpret_cast<bool*>(packet.outputBuffer) = result;
    }
}
void SQLDomains::setStringCMD(FURCMDPacket& packet){
    auto* context = reinterpret_cast<setStringCMDContext*>(packet.payload);
    auto it = sqlDomains.find(context->domainId);
    if (it != sqlDomains.end()){
        bool result = it->second->setString(context->key, context->value);
        *reinterpret_cast<bool*>(packet.outputBuffer) = result;
    }
}
void SQLDomains::getStringCMD(FURCMDPacket& packet){
    auto* context = reinterpret_cast<getStringCMDContext*>(packet.payload);
    auto it = sqlDomains.find(context->domainId);
    if (it != sqlDomains.end()){
        size_t result = it->second->getString(context->key, reinterpret_cast<char*>(packet.outputBuffer), context->bufferSize);
        *reinterpret_cast<size_t*>(packet.outputBuffer) = result;
    }
}
void SQLDomains::existsCMD(FURCMDPacket& packet){
    auto* context = reinterpret_cast<existsCMDContext*>(packet.payload);
    auto it = sqlDomains.find(context->domainId);
    if (it != sqlDomains.end()){
        bool result = it->second->exists(context->key);
        *reinterpret_cast<bool*>(packet.outputBuffer) = result;
    }
}
void SQLDomains::closeCMD(FURCMDPacket& packet){
    auto* context = reinterpret_cast<closeCMDContext*>(packet.payload);
    auto it = sqlDomains.find(context->domainId);
    if (it != sqlDomains.end()){
        it->second->close();
    }
}
void SQLDomains::isOpenCMD(FURCMDPacket& packet){
    auto* context = reinterpret_cast<isOpenCMDContext*>(packet.payload);
    auto it = sqlDomains.find(context->domainId);
    if (it != sqlDomains.end()){
        bool result = it->second->isOpen();
        *reinterpret_cast<bool*>(packet.outputBuffer) = result;
    }
}