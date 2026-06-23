#pragma once
#include "FURCMD/FURCMD.h"
#include <string>

/**
 * ContainerLoader (Container Loader) - Core Component.
 * Responsible for loading and managing active containers.
 */
class ContainerLoader {
public:
    /**
     * Initializes the ContainerLoader.
     */
    ContainerLoader();
    ~ContainerLoader() = default;

private:
    /**
     * Loads the currently active container.
     */
    void loadActiveContainer();
    /**
     * Reloads the command packet for the container.
     * 
     * @param packet The command packet to reload.
     */
    static void reloadCMD(FURCMDPacket& packet);
    /**
     * Gets the name of the active container.
     * 
     * @return The name of the active container as a string.
     */
    std::string getActiveContainerName();
};
