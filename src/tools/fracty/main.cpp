#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include "json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

class Fracty {
private:
    fs::path projectRoot;
    fs::path modulesRoot;
    fs::path templatePath;

    using CommandHandler = std::function<void(const std::vector<std::string>&)>;
    std::unordered_map<std::string, CommandHandler> commandRegistry;

public:
    Fracty() {
        projectRoot = fs::current_path();
        modulesRoot = projectRoot.parent_path();

        std::cout << "Fractal Toolchain (fracty) v0.0.10" << std::endl;
        std::cout << "Project Root: " << projectRoot << std::endl;
        std::cout << "Modules Root: " << modulesRoot << std::endl;
        std::cout << "-----------------------------------" << std::endl;

        registerCommands();
    }

    void printHelp(const std::string& command = "") {
        if (command.empty()) {
            std::cout << "Usage: fracty <command> [args]\n";
            std::cout << "Available commands (use 'fracty <command> --help' for details):\n";
            std::cout << "  engine-build, list-mods, info-mod, scan, init-mod, init-cont, info-cont, add-mod, rem-mod, build, run, cp-cont, rm-cont, arc-cont, unarc-cont, test, help\n";
        } else {
            if (command == "engine-build") std::cout << "Usage: engine-build [--recache]\n  Build the main Fractal Engine core. Use --recache to clear CMake cache.\n";
            else if (command == "list-mods") std::cout << "Usage: list-mods\n  List all available modules in the modules root.\n";
            else if (command == "info-mod") std::cout << "Usage: info-mod <name>\n  Show detailed info about a module.\n";
            else if (command == "scan") std::cout << "Usage: scan\n  Scan for modules and update project DB.\n";
            else if (command == "init-mod") std::cout << "Usage: init-mod <name>\n  Create a new module from template.\n";
            else if (command == "init-cont") std::cout << "Usage: init-cont <name>\n  Create a new container.\n";
            else if (command == "info-cont") std::cout << "Usage: info-cont <name>\n  Show detailed info about a container.\n";
            else if (command == "add-mod") std::cout << "Usage: add-mod <cont> <mod>\n  Add module to container load order.\n";
            else if (command == "rem-mod") std::cout << "Usage: rem-mod <cont> <mod>\n  Remove module from container.\n";
            else if (command == "build") std::cout << "Usage: build <cont> [--update-assets] [--src-included] [--recache]\n  Build and pack modules into container.\n";
            else if (command == "run") std::cout << "Usage: run <cont>\n  Set active container and launch engine.\n";
            else if (command == "cp-cont") std::cout << "Usage: cp-cont <src> <dest>\n  Clone a container.\n";
            else if (command == "rm-cont") std::cout << "Usage: rm-cont <name>\n  Delete a container.\n";
            else if (command == "arc-cont") std::cout << "Usage: arc-cont <name>\n  Archive container to .tar.gz.\n";
            else if (command == "unarc-cont") std::cout << "Usage: unarc-cont <path>\n  Unpack container archive.\n";
            else if (command == "test") std::cout << "Usage: test\n  Run SDK integration test.\n";
            else if (command == "fracty-build") std::cout << "Usage: fracty-build\n  Rebuild the fracty toolchain executable.\n";
            else std::cout << "Detailed help for '" << command << "' is not yet implemented.\n";
        }
    }

    bool runCommand(const std::string& cmd) {
        std::cout << "Executing: " << cmd << std::endl;
        int result = std::system(cmd.c_str());
        return result == 0;
    }

    // --- Logic Implementations ---

    void doEngineBuild(const std::vector<std::string>& args) {
        std::cout << "Building Fractal Engine Core..." << std::endl;
        fs::path buildPath = projectRoot / "build";
        bool recache = std::find(args.begin(), args.end(), "--recache") != args.end();
        try {
            if (recache && fs::exists(buildPath)) fs::remove_all(buildPath);
            if (!fs::exists(buildPath)) fs::create_directories(buildPath);
            std::string cmakeConfig = "cmake -S " + projectRoot.string() + " -B " + buildPath.string() + " -DCMAKE_BUILD_TYPE=Release";
            if (!runCommand(cmakeConfig)) return;
            std::string cmakeBuild = "cmake --build " + buildPath.string() + " --parallel $(nproc)";
            if (!runCommand(cmakeBuild)) return;
            std::cout << "Fractal Engine core built successfully.\n";
        } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; }
    }

    void doListMods(const std::vector<std::string>& args) {
        std::cout << "Listing all available modules...\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << "ID\t\t| Name\t\t\t| Version\n";
        std::cout << "------------------------------------------------------------\n";
        if (!fs::exists(modulesRoot)) return;
        int count = 0;
        for (const auto& entry : fs::directory_iterator(modulesRoot)) {
            if (entry.is_directory() && entry.path() != projectRoot) {
                fs::path jsonPath = entry.path() / "module.json";
                if (fs::exists(jsonPath)) {
                    try {
                        std::ifstream file(jsonPath);
                        json mJson; file >> mJson;
                        std::cout << mJson.value("id", "unknown") << "\t\t| " << mJson.value("name", "unknown") << "\t\t| " << mJson.value("version", "0.0.0") << "\n";
                        count++;
                    } catch (...) {}
                }
            }
        }
        std::cout << "------------------------------------------------------------\n";
        std::cout << "Total modules found: " << count << std::endl;
    }

    void doInfoMod(const std::vector<std::string>& args) {
        if (args.empty()) { std::cerr << "Error: Module name required.\n"; return; }
        fs::path jsonPath = modulesRoot / args[0] / "module.json";
        if (!fs::exists(jsonPath)) { std::cerr << "Error: Module '" << args[0] << "' not found.\n"; return; }
        try {
            std::ifstream file(jsonPath);
            json mJson; file >> mJson;
            std::cout << "\n--- Module Information ---\n";
            std::cout << "ID:           " << mJson.value("id", "N/A") << "\n";
            std::cout << "Name:         " << mJson.value("name", "N/A") << "\n";
            std::cout << "Version:      " << mJson.value("version", "N/A") << "\n";
            std::cout << "Assets Root:  " << mJson.value("assets_root", "N/A") << "\n";
            std::cout << "Config Root:  " << mJson.value("config_root", "N/A") << "\n";
            std::cout << "Entry Point:  " << mJson.value("entry_point", "N/A") << "\n";
            std::cout << "Dependencies: ";
            if (mJson.contains("dependencies") && mJson["dependencies"].is_array()) {
                for (auto& dep : mJson["dependencies"]) std::cout << dep.get<std::string>() << " ";
            } else { std::cout << "None"; }
            std::cout << "\n--------------------------\n";
        } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; }
    }

    void doScan(const std::vector<std::string>& args) {
        std::cout << "Scanning for modules in " << modulesRoot << "...\n";
        int count = 0;
        if (fs::exists(modulesRoot)) {
            for (const auto& entry : fs::directory_iterator(modulesRoot)) {
                if (entry.is_directory() && entry.path() != projectRoot) {
                    if (fs::exists(entry.path() / "module.json")) count++;
                }
            }
        }
        std::cout << "Found " << count << " valid modules. Project DB updated.\n";
    }

    void doInitMod(const std::vector<std::string>& args) {
        if (args.empty()) { std::cerr << "Error: Module name required.\n"; return; }
        std::string name = args[0];
        fs::path modPath = modulesRoot / name;
        if (fs::exists(modPath)) { std::cerr << "Error: Module directory already exists.\n"; return; }
        try {
            fs::path archivePath = projectRoot / "assets" / "module_template.tar.gz";
            if (fs::exists(archivePath)) {
                fs::create_directories(modPath);
                std::string cmd = "tar -xzvf " + archivePath.string() + " -C " + modPath.string();
                if (runCommand(cmd)) {
                    if (fs::exists(modPath / "build")) fs::remove_all(modPath / "build");
                    if (fs::exists(modPath / "build-windows-clang")) fs::remove_all(modPath / "build-windows-clang");
                    fs::create_directories(modPath / "assets");
                    fs::create_directories(modPath / "configs");
                    json modJson = {{"id", name}, {"name", name}, {"version", "1.0.0"}, {"assets_root", "assets/"}, {"config_root", "configs/"}, {"dependencies", json::array()}, {"entry_point", name + ".so"}};
                    std::ofstream file(modPath / "module.json");
                    file << modJson.dump(4);
                    
                    fs::path cmakeLists = modPath / "CMakeLists.txt";
                    if (fs::exists(cmakeLists)) {
                        std::ifstream tpl(cmakeLists);
                        std::string content((std::istreambuf_iterator<char>(tpl)), std::istreambuf_iterator<char>());
                        size_t pos = 0;
                        while((pos = content.find("ExampleModule", pos)) != std::string::npos) {
                            content.replace(pos, 13, name);
                            pos += name.length();
                        }
                        std::ofstream out(cmakeLists);
                        out << content;
                    }
                    std::cout << "Module '" << name << "' created from archive.\n";
                    return;
                }
            }
            
            fs::create_directories(modPath / "src");
            fs::create_directories(modPath / "assets");
            fs::create_directories(modPath / "configs");
            json modJson = {{"id", name}, {"name", name}, {"version", "1.0.0"}, {"assets_root", "assets/"}, {"config_root", "configs/"}, {"dependencies", json::array()}, {"entry_point", name + ".so"}};
            std::ofstream file(modPath / "module.json");
            file << modJson.dump(4);
            std::cout << "Module '" << name << "' created with basic structure.\n";
        } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; }
    }

    void doInitCont(const std::vector<std::string>& args) {
        if (args.empty()) { std::cerr << "Error: Container name required.\n"; return; }
        std::string name = args[0];
        fs::path contPath = projectRoot / "container" / name;
        try {
            fs::create_directories(contPath);
            json contJson = {{"container_id", name}, {"name", name}, {"version", "1.0.0"}, {"load_order", json::array()}, {"metadata", {{"target_platform", "Linux"}, {"build_type", "Debug"}}}, {"global_settings", {{"log_level", "Verbose"}}}};
            std::ofstream file(contPath / "container.json");
            file << contJson.dump(4);
            std::cout << "Container '" << name << "' initialized.\n";
        } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; }
    }

    void doInfoCont(const std::vector<std::string>& args) {
        if (args.empty()) { std::cerr << "Error: Container name required.\n"; return; }
        fs::path jsonPath = projectRoot / "container" / args[0] / "container.json";
        if (!fs::exists(jsonPath)) { std::cerr << "Error: Container '" << args[0] << "' not found.\n"; return; }
        try {
            std::ifstream file(jsonPath);
            json cJson; file >> cJson;
            std::cout << "\n--- Container Information ---\n";
            std::cout << "ID:           " << cJson.value("container_id", "N/A") << "\n";
            std::cout << "Name:         " << cJson.value("name", "N/A") << "\n";
            std::cout << "Version:      " << cJson.value("version", "N/A") << "\n";
            std::cout << "Load Order:\n";
            if (cJson.contains("load_order") && cJson["load_order"].is_array()) {
                int i = 1;
                for (auto& mod : cJson["load_order"]) std::cout << "  " << i++ << ". " << mod.get<std::string>() << "\n";
            } else { std::cout << "  None\n"; }
            std::cout << "Metadata:\n";
            if (cJson.contains("metadata")) {
                for (auto& [key, val] : cJson["metadata"].items()) std::cout << "  " << key << ": " << val << "\n";
            }
            std::cout << "-----------------------------\n";
        } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; }
    }

    void doAddMod(const std::vector<std::string>& args) {
        if (args.size() < 2) { std::cerr << "Error: Container and Module names required.\n"; return; }
        std::string contName = args[0];
        std::string modId = args[1];
        fs::path contJsonPath = projectRoot / "container" / contName / "container.json";
        if (!fs::exists(contJsonPath)) return;
        try {
            std::ifstream inFile(contJsonPath);
            json contJson; inFile >> contJson; inFile.close();
            std::vector<std::string> order = contJson["load_order"].get<std::vector<std::string>>();
            if (std::find(order.begin(), order.end(), modId) == order.end()) {
                contJson["load_order"].push_back(modId);
                std::ofstream outFile(contJsonPath);
                outFile << contJson.dump(4);
                std::cout << "Module '" << modId << "' added to container '" << contName << "'.\n";
            }
        } catch (...) {}
    }

    void doRemMod(const std::vector<std::string>& args) {
        if (args.size() < 2) { std::cerr << "Error: Container and Module names required.\n"; return; }
        std::string contName = args[0];
        std::string modId = args[1];
        fs::path contJsonPath = projectRoot / "container" / contName / "container.json";
        if (!fs::exists(contJsonPath)) return;
        try {
            std::ifstream inFile(contJsonPath);
            json contJson; inFile >> contJson; inFile.close();
            std::vector<std::string> order = contJson["load_order"].get<std::vector<std::string>>();
            auto it = std::find(order.begin(), order.end(), modId);
            if (it != order.end()) {
                order.erase(it);
                contJson["load_order"] = order;
                std::ofstream outFile(contJsonPath);
                outFile << contJson.dump(4);
                std::cout << "Module '" << modId << "' removed from container '" << contName << "'.\n";
            }
        } catch (...) {}
    }

    void doBuild(const std::vector<std::string>& args) {
        if (args.empty()) { std::cerr << "Error: Container name required.\n"; return; }
        std::string name = args[0];
        bool updateAssets = false;
        bool srcIncluded = false;
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i] == "--update-assets") updateAssets = true;
            if (args[i] == "--src-included") srcIncluded = true;
        }
        std::cout << "Building container '" << name << "'..." << std::endl;
        fs::path contPath = projectRoot / "container" / name;
        fs::path contJsonPath = contPath / "container.json";
        if (!fs::exists(contJsonPath)) return;
        try {
            std::ifstream inFile(contJsonPath);
            json contJson; inFile >> contJson; inFile.close();
            std::vector<std::string> loadOrder = contJson["load_order"].get<std::vector<std::string>>();
            fs::create_directories(contPath / "bin");
            fs::create_directories(contPath / "assets");
            if (srcIncluded) fs::create_directories(contPath / "src");
            for (const auto& modId : loadOrder) {
                fs::path modPath = modulesRoot / modId;
                fs::path modJsonPath = modPath / "module.json";
                if (!fs::exists(modJsonPath)) continue;
                std::ifstream mFile(modJsonPath);
                json mJson; mFile >> mJson; mFile.close();
                fs::path buildPath = modPath / "build";
                bool recache = std::find(args.begin(), args.end(), "--recache") != args.end();
                if (recache || !fs::exists(buildPath) || !fs::exists(buildPath / "CMakeCache.txt")) {
                    if (recache && fs::exists(buildPath)) fs::remove_all(buildPath);
                    fs::create_directories(buildPath);
                    if (!runCommand("cmake -S " + modPath.string() + " -B " + buildPath.string() + " -DCMAKE_BUILD_TYPE=Release")) continue;
                }
                runCommand("cmake --build " + buildPath.string() + " --parallel $(nproc)");
                std::string entryPoint = mJson.value("entry_point", modId + ".so");
                
                // Try to find the built library
                fs::path builtLib;
                
                // 1. Try exact match
                if (fs::exists(buildPath / entryPoint)) builtLib = buildPath / entryPoint;
                // 2. Try with 'lib' prefix
                else if (fs::exists(buildPath / ("lib" + entryPoint))) builtLib = buildPath / ("lib" + entryPoint);
                // 3. Fallback to any .so file (hack for ExampleModule naming mismatches)
                else {
                    for (const auto& entry : fs::directory_iterator(buildPath)) {
                        if (entry.path().extension() == ".so") {
                            builtLib = entry.path();
                            break;
                        }
                    }
                }

                if (!builtLib.empty()) {
                    fs::copy_file(builtLib, contPath / "bin" / entryPoint, fs::copy_options::overwrite_existing);
                } else {
                    std::cerr << "Error: Could not find any .so file in " << buildPath << std::endl;
                    continue;
                }
                if (updateAssets) {
                    std::string assetsRoot = mJson.value("assets_root", "assets/");
                    fs::copy(modPath / assetsRoot, contPath / "assets" / modId, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                }
                if (srcIncluded) {
                    fs::path destSrc = contPath / "src" / modId;
                    fs::copy(modPath, destSrc, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                    if (fs::exists(destSrc / "build")) fs::remove_all(destSrc / "build");
                }
            }
            std::cout << "Container '" << name << "' built successfully!" << std::endl;
        } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; }
    }

    void doRun(const std::vector<std::string>& args) {
        std::string name;
        if (args.empty()) {
            fs::path activeContFile = projectRoot / "active_container.json";
            if (fs::exists(activeContFile)) {
                try {
                    std::ifstream file(activeContFile);
                    json active; file >> active;
                    name = active.value("active_container", "");
                } catch (...) {}
            }
            if (name.empty()) {
                std::cerr << "Error: No active container set. Use 'run <container>' to set and launch.\n";
                return;
            }
        } else {
            name = args[0];
            fs::path activeContFile = projectRoot / "active_container.json";
            json active = {{"active_container", name}};
            std::ofstream file(activeContFile);
            file << active.dump(4);
            std::cout << "Active container set to " << name << ". Writing to: " << activeContFile << "\n";
        }

        std::cout << "Launching engine with container: " << name << "...\n";
        fs::path appPath = projectRoot / "build" / "fractal_app";
        if (fs::exists(appPath)) {
            runCommand(appPath.string());
        } else {
            std::cerr << "Error: Engine executable not found at " << appPath << ". Build it first with 'engine-build'.\n";
        }
    }

    void doCpCont(const std::vector<std::string>& args) {
        if (args.size() < 2) { std::cerr << "Error: Src and Dest required.\n"; return; }
        fs::copy(projectRoot / "container" / args[0], projectRoot / "container" / args[1], fs::copy_options::recursive);
        std::cout << "Container cloned successfully.\n";
    }

    void doRmCont(const std::vector<std::string>& args) {
        if (args.empty()) { std::cerr << "Error: Name required.\n"; return; }
        fs::remove_all(projectRoot / "container" / args[0]);
        std::cout << "Container deleted.\n";
    }

    void doArcCont(const std::vector<std::string>& args) {
        if (args.empty()) { std::cerr << "Error: Name required.\n"; return; }
        std::string name = args[0];
        std::string cmd = "tar -czvf " + name + ".tar.gz -C " + (projectRoot / "container").string() + " " + name;
        runCommand(cmd);
    }

    void doUnarcCont(const std::vector<std::string>& args) {
        if (args.empty()) { std::cerr << "Error: Path required.\n"; return; }
        fs::path archivePath(args[0]);
        std::string filename = archivePath.filename().string();
        std::string containerName = filename;
        size_t pos = filename.find(".tar.gz");
        if (pos != std::string::npos) containerName = filename.substr(0, pos);
        std::string cmd = "tar -xzvf " + archivePath.string() + " -C " + (projectRoot / "container").string();
        runCommand(cmd);
        std::cout << "Container '" << containerName << "' unpacked.\n";
    }

    void doFractyBuild(const std::vector<std::string>& args) {
        std::cout << "Rebuilding Fractal Toolchain (fracty)..." << std::endl;
        fs::path buildPath = projectRoot / "build";
        bool recache = std::find(args.begin(), args.end(), "--recache") != args.end();
        
        if (recache && fs::exists(buildPath)) fs::remove_all(buildPath);
        if (!fs::exists(buildPath)) fs::create_directories(buildPath);
        
        std::string cmakeConfig = "cmake -S " + projectRoot.string() + " -B " + buildPath.string() + " -DCMAKE_BUILD_TYPE=Release";
        if (!runCommand(cmakeConfig)) return;
        
        std::string buildCmd = "cmake --build " + buildPath.string() + " --target fracty";
        if (runCommand(buildCmd)) {
            std::cout << "Fracty rebuilt successfully." << std::endl;
        } else {
            std::cerr << "Failed to rebuild fracty." << std::endl;
        }
    }
    void doTest(const std::vector<std::string>& args) {
        std::cout << "Starting SDK Integration Test...\n";
        std::string testCont = "SDK_Test_Cont";
        
        fs::path buildPath = projectRoot / "build";
        if (fs::exists(buildPath)) fs::remove_all(buildPath);

        fs::path contPath = projectRoot / "container" / testCont;
        if (fs::exists(contPath)) fs::remove_all(contPath);
        fs::create_directories(contPath);
        
        json contJson = {{"container_id", testCont}, {"name", testCont}, {"version", "1.0.0"}, {"load_order", json::array()}, {"metadata", {{"target_platform", "Linux"}, {"build_type", "Debug"}}}, {"global_settings", {{"log_level", "Verbose"}}}};
        std::ofstream file(contPath / "container.json");
        file << contJson.dump(4);
        file.close();

        std::ifstream inFile(contPath / "container.json");
        json cJson; inFile >> cJson; inFile.close();
        cJson["load_order"].push_back("TestModule");
        std::ofstream outFile(contPath / "container.json");
        outFile << cJson.dump(4);
        outFile.close();

        doBuild({testCont});
        
        fs::path binPath = projectRoot / "container" / testCont / "bin" / "TestModule.so";
        if (fs::exists(binPath)) {
            std::cout << "SUCCESS: TestModule.so was built and packed into container.\n";
        } else {
            std::cerr << "FAILURE: TestModule.so not found in container.\n";
        }
        fs::remove_all(contPath);
        std::cout << "SDK Integration Test Completed.\n";
    }

    void registerCommands() {
        commandRegistry["engine-build"] = [this](auto& args) { doEngineBuild(args); };
        commandRegistry["fracty-build"] = [this](auto& args) { doFractyBuild(args); };
        commandRegistry["list-mods"] = [this](auto& args) { doListMods(args); };
        commandRegistry["info-mod"] = [this](auto& args) { doInfoMod(args); };
        commandRegistry["scan"] = [this](auto& args) { doScan(args); };
        commandRegistry["init-mod"] = [this](auto& args) { doInitMod(args); };
        commandRegistry["init-module"] = [this](auto& args) { doInitMod(args); };
        commandRegistry["init-cont"] = [this](auto& args) { doInitCont(args); };
        commandRegistry["init-container"] = [this](auto& args) { doInitCont(args); };
        commandRegistry["info-cont"] = [this](auto& args) { doInfoCont(args); };
        commandRegistry["add-mod"] = [this](auto& args) { 
            if (args.size() >= 2) {
                fs::path contJsonPath = projectRoot / "container" / args[0] / "container.json";
                if (fs::exists(contJsonPath)) {
                    std::ifstream inFile(contJsonPath);
                    json contJson; inFile >> contJson; inFile.close();
                    std::vector<std::string> order = contJson["load_order"].get<std::vector<std::string>>();
                    if (std::find(order.begin(), order.end(), args[1]) == order.end()) {
                        contJson["load_order"].push_back(args[1]);
                        std::ofstream outFile(contJsonPath);
                        outFile << contJson.dump(4);
                        std::cout << "Module '" << args[1] << "' added to container '" << args[0] << "'.\n";
                    }
                }
            } else { std::cerr << "Error: Container and Module names required.\n"; }
        };
        commandRegistry["add-module"] = [this](auto& args) { 
            if (args.size() >= 2) {
                fs::path contJsonPath = projectRoot / "container" / args[0] / "container.json";
                if (fs::exists(contJsonPath)) {
                    std::ifstream inFile(contJsonPath);
                    json contJson; inFile >> contJson; inFile.close();
                    std::vector<std::string> order = contJson["load_order"].get<std::vector<std::string>>();
                    if (std::find(order.begin(), order.end(), args[1]) == order.end()) {
                        contJson["load_order"].push_back(args[1]);
                        std::ofstream outFile(contJsonPath);
                        outFile << contJson.dump(4);
                        std::cout << "Module '" << args[1] << "' added to container '" << args[0] << "'.\n";
                    }
                }
            } else { std::cerr << "Error: Container and Module names required.\n"; }
        };
        commandRegistry["rem-mod"] = [this](auto& args) { 
            if (args.size() >= 2) {
                fs::path contJsonPath = projectRoot / "container" / args[0] / "container.json";
                if (fs::exists(contJsonPath)) {
                    std::ifstream inFile(contJsonPath);
                    json contJson; inFile >> contJson; inFile.close();
                    std::vector<std::string> order = contJson["load_order"].get<std::vector<std::string>>();
                    auto it = std::find(order.begin(), order.end(), args[1]);
                    if (it != order.end()) {
                        order.erase(it);
                        contJson["load_order"] = order;
                        std::ofstream outFile(contJsonPath);
                        outFile << contJson.dump(4);
                        std::cout << "Module '" << args[1] << "' removed from container '" << args[0] << "'.\n";
                    }
                }
            } else { std::cerr << "Error: Container and Module names required.\n"; }
        };
        commandRegistry["rem-module"] = [this](auto& args) { 
            if (args.size() >= 2) {
                fs::path contJsonPath = projectRoot / "container" / args[0] / "container.json";
                if (fs::exists(contJsonPath)) {
                    std::ifstream inFile(contJsonPath);
                    json contJson; inFile >> contJson; inFile.close();
                    std::vector<std::string> order = contJson["load_order"].get<std::vector<std::string>>();
                    auto it = std::find(order.begin(), order.end(), args[1]);
                    if (it != order.end()) {
                        order.erase(it);
                        contJson["load_order"] = order;
                        std::ofstream outFile(contJsonPath);
                        outFile << contJson.dump(4);
                        std::cout << "Module '" << args[1] << "' removed from container '" << args[0] << "'.\n";
                    }
                }
            } else { std::cerr << "Error: Container and Module names required.\n"; }
        };
        commandRegistry["build"] = [this](auto& args) { 
            if (args.empty()) return;
            bool assets = std::find(args.begin(), args.end(), "--update-assets") != args.end();
            bool src = std::find(args.begin(), args.end(), "--src-included") != args.end();
            doBuild(args);
        };
        commandRegistry["build-container"] = [this](auto& args) { 
            if (args.empty()) return;
            doBuild(args);
        };
        commandRegistry["run"] = [this](auto& args) { doRun(args); };
        commandRegistry["run-container"] = [this](auto& args) { doRun(args); };
        commandRegistry["cp-cont"] = [this](auto& args) { if (args.size() >= 2) doCpCont(args); };
        commandRegistry["copy-container"] = [this](auto& args) { if (args.size() >= 2) doCpCont(args); };
        commandRegistry["rm-cont"] = [this](auto& args) { if (!args.empty()) doRmCont(args); };
        commandRegistry["remove-container"] = [this](auto& args) { if (!args.empty()) doRmCont(args); };
        commandRegistry["arc-cont"] = [this](auto& args) { if (!args.empty()) doArcCont(args); };
        commandRegistry["archive-container"] = [this](auto& args) { if (!args.empty()) doArcCont(args); };
        commandRegistry["unarc-cont"] = [this](auto& args) { if (!args.empty()) doUnarcCont(args); };
        commandRegistry["unarchive-container"] = [this](auto& args) { if (!args.empty()) doUnarcCont(args); };
        commandRegistry["test"] = [this](auto& args) { doTest(args); };
    }

    int run(int argc, char* argv[]) {
        if (argc < 2) {
            std::string input;
            while (true) {
                std::cout << "fracty> ";
                std::getline(std::cin, input);
                if (input == "exit" || input == "quit") break;
                if (input.empty()) continue;
                
                std::vector<std::string> parts;
                size_t pos = 0;
                while ((pos = input.find(' ')) != std::string::npos) {
                    parts.push_back(input.substr(0, pos));
                    input.erase(0, pos + 1);
                }
                parts.push_back(input);
                
                std::string cmdName = parts[0];
                std::vector<std::string> args(parts.begin() + 1, parts.end());
                
                if (cmdName == "help") { printHelp(); }
                else if (!args.empty() && args.back() == "--help") {
                    printHelp(cmdName);
                } else {
                    auto it = commandRegistry.find(cmdName);
                    if (it != commandRegistry.end()) it->second(args);
                    else std::cout << "Unknown command: " << cmdName << "\n";
                }
            }
            return 0;
        }
        std::string cmdName = argv[1];
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) args.push_back(argv[i]);
        if (cmdName == "help" || cmdName == "---help") {
            printHelp((args.empty() ? "" : args[0]));
        } else if (!args.empty() && (args[0] == "--help")) {
            printHelp(cmdName);
        } else {
            auto it = commandRegistry.find(cmdName);
            if (it != commandRegistry.end()) it->second(args);
            else { std::cout << "Unknown command: " << cmdName << "\n"; printHelp(); }
        }
        return 0;
    }
};

int main(int argc, char* argv[]) {
    Fracty app;
    app.run(argc, argv);
    return 0;
}
