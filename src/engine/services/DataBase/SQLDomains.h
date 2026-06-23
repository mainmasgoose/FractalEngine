#pragma once
#include "SQLite.h"
#include "FURCMD/FURCMD.h"
#include "hash/hash.h"
#include "ankerl/unordered_dense.h"
#include <cstdint>

/**
 * registerSQLDomainContext - Context for registering a SQL domain.
 */
struct registerSQLDomainContext {
    uint32_t domainId;

    const char* domainName;
};

/**
 * openCMDContext - Context for opening a SQL database.
 */
struct openCMDContext {

    uint32_t domainId;

    const char* dbPath;

};

/**
 * executeCMDContext - Context for executing a SQL command.
 */
struct executeCMDContext {
    uint32_t domainId;

    const char* sql;
};

/**
 * setStringCMDContext - Context for setting a string value in a SQL domain.
 */
struct setStringCMDContext {
    uint32_t domainId;

    const char* key;

    const char* value;
};

/**
 * getStringCMDContext - Context for getting a string value from a SQL domain.
 */
struct getStringCMDContext {
    uint32_t domainId;

    const char* key;

    uint32_t bufferSize;

};

/**
 * existsCMDContext - Context for checking if a key exists in a SQL domain.
 */
struct existsCMDContext {
    uint32_t domainId;

    const char* key;

};

/**
 * closeCMDContext - Context for closing a SQL domain.
 */
struct closeCMDContext {
    uint32_t domainId;

};

/**
 * isOpenCMDContext - Context for checking if a SQL domain is open.
 */
struct isOpenCMDContext {
    uint32_t domainId;

};

/**
 * SQLDomains (SQL Domains) - Core Component.
 * Responsible for managing multiple SQLite database domains.
 */
class SQLDomains {
public:
    /**
     * Initializes SQLDomains.
     */
    SQLDomains();
    ~SQLDomains() = default;
    /**
     * Registers a new SQL domain.
     * 
     * @param packet The command packet containing registration data.
     */
    static void registerSQLDomain(FURCMDPacket& packet);
    /**
     * Opens a SQL database for a given domain.
     * 
     * @param packet The command packet containing open request data.
     */
    static void openCMD(FURCMDPacket& packet);
    /**
     * Executes a SQL command in a given domain.
     * 
     * @param packet The command packet containing the SQL command.
     */
    static void executeCMD(FURCMDPacket& packet);
    /**
     * Sets a string value in a given domain.
     * 
     * @param packet The command packet containing the key and value.
     */
    static void setStringCMD(FURCMDPacket& packet);
    /**
     * Gets a string value from a given domain.
     * 
     * @param packet The command packet containing the key and buffer.
     */
    static void getStringCMD(FURCMDPacket& packet);
    /**
     * Checks if a key exists in a given domain.
     * 
     * @param packet The command packet containing the key.
     */
    static void existsCMD(FURCMDPacket& packet);
    /**
     * Closes a SQL database for a given domain.
     * 
     * @param packet The command packet containing the domain ID.
     */
    static void closeCMD(FURCMDPacket& packet);
    /**
     * Checks if a SQL database is open for a given domain.
     * 
     * @param packet The command packet containing the domain ID.
     */
    static void isOpenCMD(FURCMDPacket& packet);
private:
    static ankerl::unordered_dense::map<uint32_t, SQLiteDB*, IdentityHash> sqlDomains;

};
