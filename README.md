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

## 📖 Documentation

For detailed API usage and module development guides, see:
*   [API Documentation](include/Engine/API_DOCUMENTATION.md)
*   [Russian API Documentation](include/Engine/API_DOCUMENTATION_RU.md)
