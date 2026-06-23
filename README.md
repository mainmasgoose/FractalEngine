# FractalEngine

A high-performance, modular C++23 engine built on the principles of **Command Routing** and **ABI Stability**.

## 🚀 Engine Architecture

FractalEngine is designed to be highly decoupled. Communication between modules and core services is handled by the **Fast Utility Router (FUR)**.

```text
[ External Modules ] <--- [ SDK Headers ]
      |
      v
[ FUR Command Router ] (Buffered / Asynchronous)
      |
      v
[ Internal Core Services ]
  |-- ECS (Component Management + Locking)
  |-- SmartScheduler (Cyclic/Delayed Tasks)
  |-- SystemExecution (Automated ECS Loops)
  |-- WorkScheduler (Multithreaded Tasks)
  |-- EventBus (Global Events)
  |-- SQLDB (Persistence)
  |-- VFS (Virtual File System)
  |-- MemoryAlloc (Custom Memory Management)
  |-- ModuleLoader (Dynamic Module Loading)
  |-- ContainerLoader (Resource Container Handling)
  |-- Clock (Precision Timing)
```

### Key Principles

1.  **Packet-Driven**: No direct function calls to core services. All requests are sent as `FURCMDPacket`s.
2.  **Core Isolation**: Internal engine headers are never exposed to modules. Only POD contexts and C-style interfaces are shared.
3.  **Concurrency First**: The engine uses a worker-thread model for command execution and a dedicated scheduler for heavy compute tasks.
4.  **ABI Stability**: By using C-entry points and avoiding STL in interfaces, modules can be loaded dynamically without binary compatibility issues.

## 🛠 Build and Run

### Build (out-of-source):

```bash
mkdir -p build
cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Run:

```bash
./fractal_app
```

## 🛠 Toolchain: fracty

FractalEngine includes `fracty`, a powerful CLI toolchain designed to automate the development lifecycle, from module creation to container orchestration.

### 🚀 Key Capabilities

`fracty` operates in two modes: **Command Line** (e.g., `./fracty build <container>`) and **Interactive Shell** (run `./fracty` without arguments).

#### 🏗 Core & Toolchain Management
- `engine-build [--recache]` — Compiles the main Fractal Engine core.
- `fracty-build` — Rebuilds the `fracty` toolchain itself.
- `test` — Executes SDK integration tests to verify system stability.

#### 📦 Module Management
- `scan` — Scans the modules directory and updates the internal project database.
- `list-mods` — Lists all discovered modules with their versions.
- `info-mod <name>` — Displays detailed metadata for a specific module.
- `init-mod <name>` — Scaffolds a new module from a template.

#### 🗃 Container Orchestration
Containers define a specific set of modules and settings to be loaded by the engine.
- `init-cont <name>` — Initializes a new container configuration.
- `add-mod <cont> <mod>` / `rem-mod <cont> <mod>` — Manages the module load order within a container.
- `build <cont> [--update-assets] [--src-included]` — Compiles all modules in the container and packs them into a deployable structure.
- `run <cont>` — Sets the active container and launches the engine.
- `info-cont <name>` — Shows the composition and metadata of a container.
- `cp-cont`, `rm-cont`, `arc-cont`, `unarc-cont` — Container lifecycle utilities (clone, delete, archive, unpack).

### 🔄 Typical Workflow

1.  **Build Core**: `./fracty engine-build`
2.  **Create Module**: `./fracty init-mod MyAwesomeModule` $\rightarrow$ *Implement code*
3.  **Setup Container**: `./fracty init-cont DevContainer` $\rightarrow$ `./fracty add-mod DevContainer MyAwesomeModule`
4.  **Deploy & Run**: `./fracty build DevContainer` $\rightarrow$ `./fracty run DevContainer`

## 📖 Documentation

For detailed API usage and module development guides, see:
*   [API Documentation](include/Engine/API_DOCUMENTATION.md)
*   [Russian API Documentation](include/Engine/API_DOCUMENTATION_RU.md)
