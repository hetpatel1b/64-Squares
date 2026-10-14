# KnightForge — CMake & SFML Windows Development Setup Guide

> **Document Type:** Team Standard Operating Procedure & Installation Manual  
> **Project:** KnightForge (C++ Chess Application)  
> **Target Audience:** Engineering Team (3 Windows Developers using Visual Studio Code)  
> **Scope:** Phase 0 — Environment Baseline, Toolchain Standardization, and SFML Integration  

---

## Table of Contents

1. [A. Document Overview](#a-document-overview)
2. [B. Prerequisites](#b-prerequisites)
3. [C. Establish the Compiler Baseline](#c-establish-the-compiler-baseline)
4. [D. Install CMake](#d-install-cmake)
5. [E. Select and Install SFML](#e-select-and-install-sfml)
6. [F. Explain SFML Modules](#f-explain-sfml-modules)
7. [G. Integrate SFML with CMake](#g-integrate-sfml-with-cmake)
8. [H. Build and Run the Project](#h-build-and-run-the-project)
9. [I. Runtime Dependencies](#i-runtime-dependencies)
10. [J. Troubleshooting Guide](#j-troubleshooting-guide)
11. [K. Three-Member Team Standardization](#k-three-member-team-standardization)
12. [L. Final Verification Checklist](#l-final-verification-checklist)

---

## A. Document Overview

### Purpose
This guide establishes a single, verified, and reproducible Windows development environment for the three-member team building **KnightForge**, a desktop chess application written in C++ and rendered using **SFML (Simple and Fast Multimedia Library)**.

### Scope
This manual covers **Phase 0 (Development Foundation)** only. It provides end-to-end instructions for configuring the compiler, CMake build system, and graphics library. It does not introduce chess rules, game logic, or graphical assets.

### Expected End Result
Following this guide guarantees that on all three teammates' computers:
1. CMake configures the project cleanly without warnings.
2. The C++17 compiler builds and links `KnightForge.exe` against SFML.
3. The executable starts, displays an 800×600 blank graphical window titled `KnightForge`, processes OS window-close events, and terminates cleanly with exit code 0.

### Conceptual Distinctions: Tool Roles
To prevent common configuration misunderstandings, understand the three distinct layers in this stack:
- **Installing CMake:** Installs a build-system generator executable (`cmake.exe`) that reads a `CMakeLists.txt` file and generates native build scripts for your compiler and generator (e.g., Ninja or Makefiles). CMake does not compile C++ code itself.
- **Installing SFML:** Obtains the SFML C++ header files (`.hpp`), binary libraries (`.a` / `.lib`), and runtime dynamic link libraries (`.dll`).
- **Integrating SFML into CMake:** Directs CMake where to locate those headers and libraries, how to compile project source files against them, and how to link the final executable. In KnightForge, this is handled **automatically via CMake `FetchContent`**, ensuring zero manual download steps for teammates.

---

## B. Prerequisites

### Required Software

| Component | Minimum Version | Verified Version | Purpose | Official Resource |
| :--- | :--- | :--- | :--- | :--- |
| **Operating System** | Windows 10/11 (64-bit) | Windows 11 (AMD64) | Host development platform | [Microsoft Windows](https://www.microsoft.com/windows) |
| **Editor / IDE** | VS Code (Recent) | 1.93+ (x64) | Source editing and terminal workflow | [VS Code Docs](https://code.visualstudio.com/docs) |
| **Version Control** | Git 2.40+ | 2.54.0 (x64) | Branching, tracking, and collaboration | [Git for Windows](https://git-scm.com/download/win) |
| **C++ Compiler** | MinGW-w64 GCC 13+ | GCC 16.2.0 (UCRT/SEH) | C++17 compilation and linking | [WinLibs](https://winlibs.com/) / [MSYS2](https://www.msys2.org/) |
| **Build System** | CMake 3.28+ | CMake 4.4.4 | Out-of-source project configuration | [CMake Documentation](https://cmake.org/documentation/) |
| **Build Generator** | Ninja 1.11+ | Ninja 1.13.2 | High-speed multi-threaded builds | [Ninja Build](https://ninja-build.org/) |
| **Graphics Library** | SFML 2.6.2 | SFML 2.6.2 (Static) | Windowing, input handling, and 2D rendering | [SFML Official](https://www.sfml-dev.org/) |

### Recommended Visual Studio Code Extensions
Install these extensions from the VS Code Marketplace:
1. **C/C++ Extension Pack** (`ms-vscode.cpptools-extension-pack`) — IntelliSense, code formatting, and GDB debugging.
2. **CMake Tools** (`ms-vscode.cmake-tools`) — Project configuration, kit selection, and one-click build integration.

### The Rule of Binary Compatibility (CRITICAL)
C++ does not have a standardized Application Binary Interface (ABI) across different compilers. **You cannot mix libraries built with different compiler distributions or configurations.**
- **Never mix MSVC and MinGW:** A library built with Microsoft Visual C++ (`cl.exe`) cannot link against object files built with MinGW GCC (`g++.exe`).
- **Architecture parity:** 64-bit code (`x86_64`) cannot link with 32-bit libraries (`i686` / `i586`).
- **C Runtime (CRT) parity:** Modern MinGW-w64 uses the Universal C Runtime (**UCRT**). Older MinGW distributions use legacy `msvcrt.dll`. Mixing them leads to unresolved symbols such as `__acrt_iob_func`.
- **Exception handling model:** Must match across the toolchain (KnightForge standardizes on **SEH** — Structured Exception Handling on 64-bit Windows).

---

## C. Establish the Compiler Baseline

Before installing or configuring CMake, verify the compiler on your system.

### Verification Commands
Open a PowerShell terminal in VS Code and run:

```powershell
g++ --version
where.exe g++
cmake --version
git --version
```

### Interpreting the Output

#### 1. Valid Modern 64-bit Compiler (Expected Target)
```text
g++.exe (MinGW-W64 x86_64-ucrt-posix-seh, built by Brecht Sanders, r1) 16.2.0
Target: x86_64-w64-mingw32
Thread model: posix
```
- `Target: x86_64-w64-mingw32` confirms genuine 64-bit architecture.
- `ucrt` confirms modern Universal C Runtime.
- `posix` confirms native C++11 `<thread>` and `<mutex>` support.

#### 2. Incompatible Legacy Compiler (Must Be Replaced)
If your terminal outputs:
```text
g++.exe (MinGW.org GCC-6.3.0-1) 6.3.0
Target: mingw32
Thread model: win32
Location: C:\MinGW\bin\g++.exe
```
This is an **obsolete 32-bit compiler from 2016**. It cannot compile modern C++17 or link with 64-bit SFML libraries.

### Installing the Agreed Compiler: WinLibs MinGW-w64

#### Method 1: Automated installation via Windows Package Manager (`winget`)
Run in PowerShell:
```powershell
winget install --id BrechtSanders.WinLibs.POSIX.UCRT -e --source winget
```

#### Method 2: Manual Standalone Archive (Zero-Installer)
1. Download the standalone 64-bit UCRT zip from [winlibs.com](https://winlibs.com/):
   - Package: `GCC 16.2.0 + MinGW-w64 14.0.0 UCRT (POSIX threads, SEH exceptions)`.
2. Extract the archive directly to `C:\mingw64` so that `C:\mingw64\bin\g++.exe` exists.
3. Add `C:\mingw64\bin` to your User Environment `Path`.

### Resolving the PATH Precedence Collision
If an old `C:\MinGW\bin` directory exists on your computer:
1. In the Windows Search Bar, type **Edit system environment variables** and press Enter.
2. Click **Environment Variables...**.
3. Under **System variables**, select `Path` and click **Edit...**.
4. Remove `C:\MinGW\bin` (or move your new `C:\mingw64\bin` entry **above** it).
5. Click **OK** to save.

---

## D. Install CMake

CMake is a cross-platform build-system generator. It reads `CMakeLists.txt` and generates build scripts for **Ninja** or **MinGW Makefiles**.

### Official Installation Procedure

#### Method 1: Via Windows Terminal (`winget` — Recommended)
Run in PowerShell:
```powershell
winget install --id Kitware.CMake -e --source winget
winget install --id Ninja-build.Ninja -e --source winget
```
*(Accept any Windows User Account Control prompt that appears).*

#### Method 2: Official MSI Installer
1. Navigate to the official download page: [cmake.org/download](https://cmake.org/download/).
2. Under **Binary distributions**, download the Windows x64 Installer:  
   `cmake-<version>-windows-x86_64.msi`.
3. Launch the installer.
4. ⚠️ **Mandatory Step during installation wizard:** On the **"Install Options"** screen, select:  
   🔘 **"Add CMake to the system PATH for all users"** (or for the current user).
5. Complete the installation wizard.

### Reloading the Terminal Environment
Windows environment variables written to the registry are **not** inherited by existing terminal sessions:
1. In VS Code, close your current terminal tab by clicking the **Trash Can 🗑️** icon.
2. Open a new terminal (`Terminal -> New Terminal` or `Ctrl + Shift + \``).
3. Alternatively, refresh the active session directly in PowerShell:
   ```powershell
   $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
   ```

### Verification
Run:
```powershell
cmake --version
where.exe cmake
ninja --version
```

**Expected Successful Output:**
```text
cmake version 4.4.4 (or 3.28+)
C:\Program Files\CMake\bin\cmake.exe
1.13.2
```

---

## E. Select and Install SFML

### Version Decision: SFML 2.6.2
The team has standardized on **SFML 2.6.2** for the following technical reasons:

| Evaluation Factor | SFML 2.6.2 (Selected) | SFML 3.x (Rejected for Phase 0) |
| :--- | :--- | :--- |
| **Maturity** | Battle-tested, stable Long-Term Support release. | Newly released; active API migrations. |
| **API Architecture** | Clean, established event and windowing loop. | Breaking changes (`sf::Event` variant, `std::optional` return). |
| **C++ Standard** | Clean C++17 compatibility. | Strict C++17 requirement with revised namespaces. |
| **Documentation & Assets** | 100% compatible with existing chess tutorials and UI guides. | Incompatible syntax with existing SFML 2.x documentation. |
| **Compiler Compatibility** | Compiles cleanly from source under GCC 14–16 with UCRT. | Requires updated toolchain dependencies. |

### Dependency Strategy: Automated CMake `FetchContent`
Rather than requiring each teammate to download arbitrary ZIP files or set up third-party package managers:
- **KnightForge uses CMake's built-in `FetchContent` module.**
- When you run `cmake -B build`, CMake automatically clones the official, tagged release of **SFML 2.6.2** from `https://github.com/SFML/SFML.git`.
- It compiles SFML from source using **your exact local compiler**, guaranteeing **100% binary, ABI, and CRT compatibility**.
- **Static Linking Default:** SFML is compiled as static libraries (`BUILD_SHARED_LIBS=OFF`), producing a self-contained executable with zero missing DLL headaches.

---

## F. Explain SFML Modules

SFML is organized into modular libraries. KnightForge requires three modules in Phase 0:

```text
SFML
├── sfml-system    (Base data types, strings, clocks, thread management)
├── sfml-window    (OS window creation, context management, mouse/keyboard events)
└── sfml-graphics  (Hardware-accelerated 2D rendering, sprites, textures, render targets)
```

### Module Roles:
1. **System (`sfml-system`):** Foundational utilities (`sf::Time`, `sf::Clock`, `sf::String`). Required by all other SFML modules.
2. **Window (`sfml-window`):** Wraps Win32 window handles, manages OpenGL contexts, and processes window-close, resize, and keypress events.
3. **Graphics (`sfml-graphics`):** Provides the `sf::RenderWindow`, 2D drawables (`sf::RectangleShape`, `sf::Sprite`), texture loaders, and colors (`sf::Color`).

> [!NOTE]
> These are **C++ library modules linked during build**, not independent external programs. You do not install them separately.

### Official CMake Imported Target Names
When linking SFML in `CMakeLists.txt`, use the official target names:
- `sfml-graphics` (or `SFML::Graphics`)
- `sfml-window`   (or `SFML::Window`)
- `sfml-system`   (or `SFML::System`)

---

## G. Integrate SFML with CMake

Below is the complete, verified [CMakeLists.txt](file:///d:/64-Squares/CMakeLists.txt) configured in the KnightForge repository root:

```cmake
cmake_minimum_required(VERSION 3.28)

# Declare C++ project identity and metadata
project(KnightForge
    VERSION 0.1.0
    DESCRIPTION "C++ Chess Application using SFML"
    LANGUAGES CXX
)

# ------------------------------------------------------------------------------
# 1. C++ Standard Specification
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# 2. Dependency Management: SFML 2.6.2
# ------------------------------------------------------------------------------
# Build SFML statically on Windows to eliminate external runtime DLL dependencies
option(BUILD_SHARED_LIBS "Build libraries as shared (DLLs on Windows)" OFF)

# Dual-mode discovery: Check local system package first; fallback to FetchContent
find_package(SFML 2.6 COMPONENTS graphics window system QUIET)

if(NOT SFML_FOUND)
    message(STATUS "SFML 2.6 not found locally. Fetching official SFML 2.6.2 release...")
    include(FetchContent)

    # Disable unused SFML modules to accelerate compilation
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "Disable SFML audio module" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "Disable SFML network module" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "Disable SFML examples" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "Disable SFML documentation" FORCE)

    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        2.6.2
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(SFML)
endif()

# ------------------------------------------------------------------------------
# 3. Application Target Definition
# ------------------------------------------------------------------------------
add_executable(KnightForge
    src/main.cpp
)

# Target-scoped header search paths
target_include_directories(KnightForge PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Target-scoped language feature requirements
target_compile_features(KnightForge PRIVATE
    cxx_std_17
)

# Target-scoped library linkage
if(TARGET sfml-graphics)
    target_link_libraries(KnightForge PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
    )
elseif(TARGET SFML::Graphics)
    target_link_libraries(KnightForge PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
    )
else()
    message(FATAL_ERROR "SFML targets could not be resolved.")
endif()

# ------------------------------------------------------------------------------
# 4. Runtime DLL Deployment (Active only when BUILD_SHARED_LIBS is ON)
# ------------------------------------------------------------------------------
if(BUILD_SHARED_LIBS)
    add_custom_command(TARGET KnightForge POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:KnightForge>
            $<TARGET_FILE_DIR:KnightForge>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime shared library DLLs to executable directory..."
    )
endif()
```

---

## H. Build and Run the Project

Follow these exact steps from the repository root:

### Step 1: Open the Repository Root
In PowerShell, verify your current working directory:
```powershell
cd D:\64-Squares    # Or your local KnightForge clone path
pwd
```

### Step 2: Ensure PATH Prioritizes 64-bit MinGW and CMake
```powershell
$env:Path = "C:\Users\hetpa\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin;C:\Program Files\CMake\bin;C:\Program Files\Git\cmd;" + $env:Path
```

### Step 3: Configure the Build Directory (Out-of-Source)
```powershell
cmake -S . -B build -G "Ninja"
```
- `-S .` points to the source root containing `CMakeLists.txt`.
- `-B build` instructs CMake to generate all build scripts in a dedicated `build/` folder.
- `-G "Ninja"` specifies the high-speed Ninja build generator.

### Step 4: Compile the Target
```powershell
cmake --build build
```
*(On initial build, Ninja will fetch SFML 2.6.2, compile the system, window, and graphics libraries, and link `KnightForge.exe` in ~20 seconds).*

### Step 5: Locate and Run the Executable
The compiled executable is placed at:
```powershell
.\build\KnightForge.exe
```

### Step 6: Verify Application Lifecycle
1. An 800×600 pixel window titled **KnightForge** appears with a dark gray background.
2. The window renders smoothly at 60 FPS without freezing or crashing.
3. Click the close button (**X** in the window title bar).
4. The process exits cleanly and control returns to the terminal prompt with exit code 0.

---

## I. Runtime Dependencies

### Linking vs. Launching: The Critical Difference
- **Linking:** Occurs at compile time when `ld.exe` connects function calls in `main.cpp` to symbol definitions in the static library archives (`libsfml-*.a`).
- **Launching:** Occurs at runtime when the Windows OS loader reads `KnightForge.exe` and attempts to load any Dynamic Link Libraries (DLLs) required by the binary into memory.

### Static vs. Shared Deployment

#### 1. KnightForge Default: Static Linking (`BUILD_SHARED_LIBS=OFF`)
Because `CMakeLists.txt` sets `BUILD_SHARED_LIBS` to `OFF` by default:
- All SFML code is baked directly inside `KnightForge.exe`.
- **Zero SFML DLLs are needed in the folder.**
- The resulting `.exe` can be copied or moved without breaking.

#### 2. Shared Linking (`BUILD_SHARED_LIBS=ON`)
If dynamic linking is enabled:
- `KnightForge.exe` depends on `sfml-graphics-2.dll`, `sfml-window-2.dll`, and `sfml-system-2.dll`.
- Our `CMakeLists.txt` includes an automated post-build command:
  ```cmake
  $<TARGET_RUNTIME_DLLS:KnightForge>
  ```
  This command automatically copies all required DLLs into `build/` next to `KnightForge.exe`.

> [!CAUTION]
> **Never copy DLLs into `C:\Windows\System32` or `C:\Windows\SysWOW64`.** Doing so pollutes the global OS path and creates catastrophic DLL-version conflicts across other software.

---

## J. Troubleshooting Guide

| Problem Symptom | Likely Cause | Diagnostic Command | Safe Corrective Action | Verification |
| :--- | :--- | :--- | :--- | :--- |
| **`cmake : The term 'cmake' is not recognized`** | CMake binary directory is missing from terminal `PATH`. | `where.exe cmake` | Restart terminal or run: `$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")` | `cmake --version` |
| **`g++ : The term 'g++' is not recognized`** | MinGW compiler is not registered in `PATH`. | `where.exe g++` | Add `C:\mingw64\bin` to User `Path` or install via winget. | `g++ --version` |
| **CMake selects 32-bit GCC 6.3 instead of 64-bit compiler** | Legacy `C:\MinGW\bin` appears in System PATH ahead of MinGW-w64. | `where.exe g++` | Remove `C:\MinGW\bin` from System Environment Variables or reorder PATH so `C:\mingw64\bin` is first. | `g++ -v` reports `Target: x86_64-w64-mingw32` |
| **`CMake Error: Generator: execution of make failed`** | Ninja build tool is missing or not reachable. | `where.exe ninja` | WinLibs includes Ninja. Ensure `C:\mingw64\bin` is in PATH or install via `winget install Ninja-build.Ninja`. | `ninja --version` |
| **`CMake Error: find_package(SFML) failed`** | SFML not installed locally and FetchContent network access blocked. | Check network connectivity to `github.com` | Ensure internet access is available on first configure so CMake can fetch `SFML/SFML.git`. | Re-run `cmake -S . -B build -G "Ninja"` |
| **`ld.exe: undefined reference to sf::RenderWindow...`** | Target failed to link SFML libraries or version mismatch. | Inspect `CMakeLists.txt` link target lines | Ensure `target_link_libraries(KnightForge PRIVATE sfml-graphics sfml-window sfml-system)` is declared. | `cmake --build build --clean-first` |
| **`ld.exe: skipping incompatible ... when searching for -lsfml`** | Architecture collision (e.g. 32-bit compiler linking 64-bit library). | `g++ -dumpmachine` | Re-run build using the verified 64-bit MinGW-w64 toolchain. | Output shows `x86_64` |
| **`System Error: sfml-graphics-2.dll was not found`** | Binary was built with dynamic linking but DLLs are missing from the folder. | `Test-Path .\build\sfml-graphics-2.dll` | Reconfigure with default static linking: `cmake -S . -B build -DBUILD_SHARED_LIBS=OFF`. | Launch `KnightForge.exe` |
| **`An Application Control policy has blocked this file`** | Windows 11 Smart App Control (SAC) blocks local unsigned development binaries. | `Get-WinEvent -LogName "Microsoft-Windows-CodeIntegrity/Operational" -MaxEvents 3` | Double-click `KnightForge.exe` in File Explorer and click "Run anyway", or enable Windows Developer Mode (`Settings -> System -> For developers -> Developer Mode: ON`). | Binary launches window |
| **CMake builds with stale or corrupt cache** | Switching generators or compilers in an existing `build/` folder. | Check `build\CMakeCache.txt` | Safely wipe the build tree: `Remove-Item -Recurse -Force build`. Do not touch repository files. | Clean `cmake -S . -B build -G "Ninja"` |

---

## K. Three-Member Team Standardization

### Agreed Team Specification

| Parameter | Team Standard Value |
| :--- | :--- |
| **Host Operating System** | Windows 10/11 64-bit (`AMD64`) |
| **IDE & Extensions** | VS Code + C/C++ Extension Pack + CMake Tools |
| **Compiler Distribution** | MinGW-w64 (WinLibs standalone distribution) |
| **Compiler Version & Architecture** | GCC 16.2.0 (`x86_64-w64-mingw32`, UCRT, POSIX threads, SEH) |
| **C++ Standard** | C++17 (`cxx_std_17`) |
| **Build System & Generator** | CMake 4.4.x (Minimum 3.28) + Ninja 1.13.x |
| **Graphics Library** | SFML 2.6.2 (Official release, compiled statically via FetchContent) |
| **Standard Build Directory** | `build/` (Root out-of-source directory, excluded in `.gitignore`) |

### Reproducing from a Fresh Clone (Teammate Onboarding)
When Teammates 2 and 3 clone the repository:
```powershell
# 1. Clone repository
git clone https://github.com/hetpatel1b/64-Squares.git KnightForge
cd KnightForge

# 2. Configure build tree (fetches SFML automatically)
cmake -S . -B build -G "Ninja"

# 3. Build executable
cmake --build build

# 4. Launch window smoke test
.\build\KnightForge.exe
```

---

## L. Final Verification Checklist

Every teammate must complete and verify each checkbox independently on their computer:

- [ ] **1. Compiler Executable Verified:** `where.exe g++` points to 64-bit MinGW-w64 (`x86_64-w64-mingw32`), NOT `C:\MinGW\bin\g++.exe`.
- [ ] **2. Compiler Version Verified:** `g++ --version` reports GCC 14.x or 16.x with UCRT and POSIX threads.
- [ ] **3. CMake Available in Terminal:** `cmake --version` returns version 3.28 or higher.
- [ ] **4. Build Generator Available:** `ninja --version` returns version 1.11 or higher.
- [ ] **5. Git Configured:** `git --version` returns Git 2.x.
- [ ] **6. Repository Structure Intact:** Required directories (`src/`, `include/`, `assets/`, `tests/`, `saves/`) and files exist.
- [ ] **7. SFML Version Standardized:** `CMakeLists.txt` declares SFML 2.6.2 via `FetchContent`.
- [ ] **8. CMake Configuration Succeeds:** `cmake -S . -B build -G "Ninja"` outputs `Configuring done` and `Generating done` with exit code 0.
- [ ] **9. Application Builds Without Errors:** `cmake --build build` produces `build/KnightForge.exe` with exit code 0.
- [ ] **10. Zero Missing Runtime DLLs:** Executable is self-contained (static linking verified).
- [ ] **11. Application Window Opens:** `KnightForge.exe` launches and renders an 800×600 dark window.
- [ ] **12. Clean Termination:** Clicking the close button terminates the process cleanly with exit code 0.

---

> **Phase 0 Sign-Off Notice:**  
> Phase 0 is officially complete only when all three teammates have independently checked off every item in Section L on their respective machines. Do not begin Phase 1 (Chess Architecture and Board Representation) until all three developers have validated their environment.
