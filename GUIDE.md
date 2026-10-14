# 64 Squares — Installing and Configuring SFML on Windows

> **Document Type:** Team Standard Operating Procedure & Technical Installation Manual  
> **Project:** 64 Squares (C++ Chess Application)  
> **Target Audience:** Engineering Team (3 Windows Developers using Visual Studio Code)  
> **Scope:** Phase 0 — Complete Development Environment, Toolchain Baseline, and SFML Integration  

---

## Table of Contents

1. [A. Prerequisites & Verification](#a-prerequisites--verification)
2. [B. Install and Bootstrap vcpkg](#b-install-and-bootstrap-vcpkg)
3. [C. Choose a Compatible SFML Version and Compiler](#c-choose-a-compatible-sfml-version-and-compiler)
4. [D. Install SFML](#d-install-sfml)
5. [E. Integrate SFML with CMake](#e-integrate-sfml-with-cmake)
6. [F. Runtime Dependencies & DLL Deployment](#f-runtime-dependencies--dll-deployment)
7. [G. Build and Run the Project](#g-build-and-run-the-project)
8. [H. Comprehensive Troubleshooting Guide](#h-comprehensive-troubleshooting-guide)
9. [I. Three-Member Team Standardization](#i-three-member-team-standardization)
10. [J. Final Verification Checklist](#j-final-verification-checklist)

---

## A. Prerequisites & Verification

Before installing SFML or configuring the build system, each teammate must verify that the underlying development tools are installed and accessible from the terminal.

### Required Software Components

| Component | Minimum Version | Verified Version | Purpose | Official Resource |
| :--- | :--- | :--- | :--- | :--- |
| **Operating System** | Windows 10/11 (64-bit) | Windows 11 (AMD64) | Host OS | [Microsoft Windows](https://www.microsoft.com/windows) |
| **Editor / IDE** | Visual Studio Code | 1.93+ (x64) | Source editing and terminal workflow | [VS Code Official](https://code.visualstudio.com/) |
| **Compiler** | MinGW-w64 GCC 13+ | GCC 16.2.0 (UCRT/SEH) | C++17 compilation and linking | [WinLibs](https://winlibs.com/) / [MSYS2](https://www.msys2.org/) |
| **Build System** | CMake 3.28+ | CMake 4.4.4 | Cross-platform build automation | [CMake Documentation](https://cmake.org/documentation/) |
| **Build Generator** | Ninja 1.11+ | Ninja 1.13.2 | High-speed multi-threaded builds | [Ninja Build](https://ninja-build.org/) |
| **Version Control** | Git 2.40+ | 2.54.0 (x64) | Repository cloning and branch tracking | [Git for Windows](https://git-scm.com/download/win) |

### Recommended VS Code Extensions
Install the following extensions from the VS Code Marketplace:
1. **C/C++ Extension Pack** (`ms-vscode.cpptools-extension-pack`) — IntelliSense, code formatting, and debugging.
2. **CMake Tools** (`ms-vscode.cmake-tools`) — Configure kits, presets, and one-click build integration.

---

### Verification Commands
Open a fresh PowerShell terminal in VS Code (`Ctrl + \``) and run:

```powershell
g++ --version
cmake --version
git --version
where.exe g++
where.exe cmake
```

#### Interpreting the Output:
- **`g++ --version`**: Must report an `x86_64-w64-mingw32` target with modern GCC (GCC 14.x or 16.x).
- **`where.exe g++`**: Must point to your 64-bit compiler (e.g., `C:\mingw64\bin\g++.exe` or WinLibs path).
  > ⚠️ **Warning:** If `C:\MinGW\bin\g++.exe` is listed first, an obsolete 32-bit compiler from 2016 is shadowing your modern compiler. Remove `C:\MinGW\bin` from your System PATH.
- **`cmake --version`**: Must report version 3.28 or higher.
- **`where.exe cmake`**: Must point to `C:\Program Files\CMake\bin\cmake.exe` (or your active CMake directory).

#### What to Do If a Command Is Not Recognized:
If PowerShell returns `The term '...' is not recognized`:
1. **Tool Not Installed:** Install the tool using `winget` (see Section C & D below).
2. **PATH Not Reloaded:** Active terminals do not automatically receive environment variable changes. Reload your PATH within the current session:
   ```powershell
   $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
   ```
   Or close the terminal tab (click the **Trash Can 🗑️** icon) and open a new terminal.

---

## B. Install and Bootstrap vcpkg

[vcpkg](https://vcpkg.io/) is Microsoft's cross-platform C++ dependency manager. It integrates with CMake to automate downloading, compiling, and linking third-party libraries.

### Step 1: Check If vcpkg Is Already Installed
In PowerShell, check if vcpkg already exists on disk:
```powershell
Test-Path "C:\vcpkg"
where.exe vcpkg
```
- If `Test-Path "C:\vcpkg"` returns `True`, vcpkg is already present. Skip to Step 3.
- If it returns `False`, proceed to Step 2.

### Step 2: Clone and Bootstrap vcpkg
Run PowerShell as Administrator (or standard user with write access to `C:\`):
```powershell
# Navigate to the root of drive C:
cd C:\

# Clone official Microsoft vcpkg repository
git clone https://github.com/microsoft/vcpkg.git

# Navigate into the vcpkg directory
cd C:\vcpkg

# Run the bootstrap script to compile the vcpkg executable
.\bootstrap-vcpkg.bat
```
*(Reference: [Official Microsoft vcpkg Documentation](https://learn.microsoft.com/en-us/vcpkg/))*

### Step 3: Verify the vcpkg Executable
```powershell
C:\vcpkg\vcpkg.exe version
```
**Expected Output:**
```text
vcpkg package management program version 2024.xx.xx / 2025.xx.xx
```

---

## C. Choose a Compatible SFML Version and Compiler

Selecting the correct SFML version and target configuration is **the most critical decision in C++ library integration**.

### SFML 2.6.2 vs. SFML 3.x

| Evaluation Factor | SFML 2.6.2 (Selected Standard) | SFML 3.x (Incompatible for Phase 0) |
| :--- | :--- | :--- |
| **API Architecture** | Stable `sf::Event` polling loop (`window.pollEvent(event)`). | Breaking change: `sf::Event` is a `std::variant`; `pollEvent()` returns `std::optional`. |
| **C++ Standard** | Clean C++17 compatibility. | Strict C++17 requirement with completely overhauled namespaces. |
| **Documentation & Code** | 100% compatible with established chess tutorials and UI patterns. | Incompatible syntax with existing SFML 2.x guides. |
| **Package Availability** | Fully mature in vcpkg and official Git repository. | Recent release; undergoing ecosystem stabilization. |

### The Critical Rule of MinGW-w64 Triplet Compatibility
In vcpkg on Windows:
- The default triplet is **`x64-windows`**, which uses **Microsoft Visual C++ (MSVC `cl.exe`)**.
- ⚠️ **DO NOT install SFML with `x64-windows`!** Doing so compiles `.lib` binaries with MSVC. Attempting to link MSVC-built SFML with your MinGW GCC compiler will fail with severe ABI and symbol collisions (`undefined reference to __imp_...`).
- When compiling with MinGW-w64 GCC, you must specify the MinGW community triplet:
  - **`x64-mingw-static` (Recommended):** Compiles static `.a` libraries. Zero external DLLs needed at runtime.
  - **`x64-mingw-dynamic`:** Compiles dynamic `.dll` libraries. Requires copying SFML DLLs to your output folder.

---

## D. Install SFML

### Method 1: Via vcpkg (Package Manager Approach)
To install the verified SFML 2.6 release for 64-bit MinGW:

```powershell
# Ensure your MinGW compiler is available on PATH:
$env:Path = "C:\mingw64\bin;C:\Program Files\CMake\bin;C:\Program Files\Git\cmd;" + $env:Path

# Install SFML using the x64-mingw-static triplet
C:\vcpkg\vcpkg.exe install sfml:x64-mingw-static
```

#### What This Installs:
vcpkg compiles and provides the three core SFML modules required by 64 Squares:
1. **`sfml-system`:** Core timing, string manipulation, and threading utilities.
2. **`sfml-window`:** OS window creation, OpenGL context management, and input events.
3. **`sfml-graphics`:** 2D rendering engine (`sf::RenderWindow`, `sf::Sprite`, `sf::Color`).

---

### Method 2: Automated CMake `FetchContent` (Built-in Project Fallback)
The 64 Squares repository is architected with a **dual-mode dependency strategy** in [CMakeLists.txt](file:///d:/64-Squares/CMakeLists.txt):
- If vcpkg (or a system SFML installation) is present, CMake finds and links it immediately.
- If vcpkg is **not** installed on a teammate's computer, CMake automatically downloads official SFML 2.6.2 via `FetchContent` and compiles it with their local compiler.
- **Teammates do not need to install anything extra** if they prefer not to set up vcpkg. Running `cmake -B build` handles everything automatically.

---

## E. Integrate SFML with CMake

### The Project `CMakeLists.txt`
Below is the complete, target-scoped [CMakeLists.txt](file:///d:/64-Squares/CMakeLists.txt) configured in the repository root:

```cmake
cmake_minimum_required(VERSION 3.28)

# Declare C++ project identity and metadata
project(sixty_four_squares
    VERSION 0.1.0
    DESCRIPTION "64 Squares — C++ Chess Application using SFML"
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
option(BUILD_SHARED_LIBS "Build libraries as shared (DLLs on Windows)" OFF)

# Attempt to locate pre-installed SFML package (supports vcpkg and system prefix)
find_package(SFML 2.6 COMPONENTS graphics window system QUIET)

if(NOT SFML_FOUND)
    message(STATUS "SFML 2.6 not found locally. Fetching official SFML 2.6.2 release...")
    include(FetchContent)

    # Disable unused modules to streamline build times
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
add_executable(sixty_four_squares
    src/main.cpp
)

# Target-scoped include paths
target_include_directories(sixty_four_squares PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Target-scoped compiler features
target_compile_features(sixty_four_squares PRIVATE
    cxx_std_17
)

# Target-scoped SFML module linkage
if(TARGET sfml-graphics)
    target_link_libraries(sixty_four_squares PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
    )
elseif(TARGET SFML::Graphics)
    target_link_libraries(sixty_four_squares PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
    )
else()
    message(FATAL_ERROR "SFML targets could not be resolved.")
endif()

# ------------------------------------------------------------------------------
# 4. Runtime DLL Deployment (Active only when dynamic linking is used)
# ------------------------------------------------------------------------------
if(BUILD_SHARED_LIBS)
    add_custom_command(TARGET sixty_four_squares POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:sixty_four_squares>
            $<TARGET_FILE_DIR:sixty_four_squares>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime shared library DLLs to executable directory..."
    )
endif()
```

---

## F. Runtime Dependencies & DLL Deployment

### Linking vs. Launching:
- **Compile/Link Time:** The linker (`ld.exe`) verifies that all SFML functions exist in the library archives (`libsfml-*.a`).
- **Runtime (Launch Time):** When `sixty_four_squares.exe` starts, Windows loads any required `.dll` files into memory.

### Static vs. Shared Linking

#### 1. Static Linking (Recommended & Default: `BUILD_SHARED_LIBS=OFF` or `x64-mingw-static`)
- All SFML machine code is bundled directly into `sixty_four_squares.exe`.
- **Zero external SFML DLLs are needed in the folder.**
- The executable can run independently without path or DLL search failures.

#### 2. Dynamic Linking (`BUILD_SHARED_LIBS=ON` or `x64-mingw-dynamic`)
- The application requires `sfml-graphics-2.dll`, `sfml-window-2.dll`, and `sfml-system-2.dll`.
- Our `CMakeLists.txt` automatically invokes `$<TARGET_RUNTIME_DLLS:sixty_four_squares>` post-build to copy these DLLs directly next to `sixty_four_squares.exe`.

> [!CAUTION]
> **Never copy DLLs into `C:\Windows\System32` or `C:\Windows\SysWOW64`.** Doing so creates severe system-wide version conflicts for other applications.

---

## G. Build and Run the Project

Execute these commands from the project root (`D:\64-Squares`):

### Option 1: Standard Build (Using Automated FetchContent)
```powershell
# 1. Ensure 64-bit MinGW toolchain is in session PATH
$env:Path = "C:\Users\hetpa\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin;C:\Program Files\CMake\bin;C:\Program Files\Git\cmd;" + $env:Path

# 2. Configure build tree using Ninja
cmake -S . -B build -G "Ninja"

# 3. Compile the application
cmake --build build

# 4. Run the executable
.\build\sixty_four_squares.exe
```

### Option 2: Build with vcpkg Integration
If building against your vcpkg installation:
```powershell
cmake -S . -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build
.\build\sixty_four_squares.exe
```

### Verification Criteria:
1. An 800×600 window titled **64 Squares** appears with a dark gray background.
2. The window renders at a stable 60 FPS without freezing.
3. Clicking the close button (**X**) terminates the process cleanly with exit code 0.

---

## H. Comprehensive Troubleshooting Guide

| Issue / Error Message | Root Cause | Diagnostic Command | Safe Corrective Action |
| :--- | :--- | :--- | :--- |
| **`cmake : The term 'cmake' is not recognized`** | CMake binary path missing from terminal session `PATH`. | `where.exe cmake` | Run `$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")` or reopen VS Code. |
| **`g++ : The term 'g++' is not recognized`** | MinGW compiler is not in `PATH`. | `where.exe g++` | Add `C:\mingw64\bin` to User `PATH` or install via winget. |
| **`vcpkg: error: Invalid triplet 'x64-windows'` when building with MinGW** | Attempted to use MSVC triplet with a MinGW compiler. | Inspect vcpkg install command | Specify the MinGW triplet: `vcpkg install sfml:x64-mingw-static`. |
| **`CMake Error: Could not find a package configuration file provided by "SFML"`** | SFML not found and network blocked for FetchContent. | Test internet connection | Ensure internet connectivity so CMake can clone `SFML/SFML.git`, or pass `-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake`. |
| **`ld.exe: skipping incompatible ... when searching for -lsfml`** | Architecture mismatch (e.g. 32-bit compiler linking 64-bit library). | `g++ -dumpmachine` | Verify compiler outputs `x86_64-w64-mingw32`. Remove any legacy `C:\MinGW\bin` entries from PATH. |
| **`System Error: sfml-graphics-2.dll was not found`** | Dynamic library built without DLLs in the output folder. | `Test-Path .\build\sfml-graphics-2.dll` | Rebuild statically (`-DBUILD_SHARED_LIBS=OFF`) or copy DLLs from `C:\vcpkg\installed\x64-mingw-dynamic\bin\` to `build\`. |
| **`no matching function for call to sf::RenderWindow::pollEvent(sf::Event&)`** | Using SFML 3 API with SFML 2 (or vice-versa). | Check `main.cpp` syntax | In SFML 2.6.2, use `while (window.pollEvent(event))`. Do not use SFML 3 `std::optional` syntax. |
| **`Program failed to run: An Application Control policy has blocked this file`** | Windows 11 Smart App Control (SAC) blocks local unsigned development binaries. | Check Windows Security event log | Double-click `sixty_four_squares.exe` in File Explorer and click **"More info" -> "Run anyway"**, or enable **Developer Mode** in Windows Settings (`Settings -> System -> For developers`). |
| **Stale or corrupt CMake build tree** | Switching compilers or triplets in an existing build folder. | Inspect `build\CMakeCache.txt` | Safely wipe and reconfigure: `Remove-Item -Recurse -Force build; cmake -S . -B build -G "Ninja"`. |

---

## I. Three-Member Team Standardization

To ensure 100% build reproducibility and eliminate machine-specific discrepancies across all three teammates:

### Master Toolchain Specification

| Component | Standardized Requirement |
| :--- | :--- |
| **Operating System** | Windows 10/11 64-bit (`AMD64`) |
| **IDE** | VS Code (with C/C++ Extension Pack & CMake Tools) |
| **Compiler** | MinGW-w64 GCC 16.2.0 (`x86_64-w64-mingw32`, UCRT runtime, POSIX threads, SEH exceptions) |
| **Language Standard** | C++17 (`cxx_std_17`) |
| **Build System & Generator** | CMake 4.4.x (Minimum 3.28) + Ninja 1.13.x |
| **Graphics Library** | SFML 2.6.2 (Compiled statically from source via CMake `FetchContent`) |
| **Executable Target** | `sixty_four_squares` (`build/sixty_four_squares.exe`) |
| **Runtime Dependencies** | None (Fully self-contained static executable) |

### Clean Onboarding Steps for Teammates 2 and 3:
1. Clone the repository: `git clone https://github.com/hetpatel1b/64-Squares.git`
2. Open the folder in VS Code: `code 64-Squares`
3. Configure the build: `cmake -S . -B build -G "Ninja"`
4. Compile the project: `cmake --build build`
5. Test the application: `.\build\sixty_four_squares.exe`

---

## J. Final Verification Checklist

Every teammate must verify each item on their local workstation. Items are marked based on real test evidence:

- [x] **1. Git, GCC/G++, and CMake work:** `git --version` (2.54), `g++ --version` (GCC 16.2.0 x86_64), and `cmake --version` (4.4.4) execute cleanly.
- [x] **2. Selected SFML Package & Compiler are Compatible:** SFML 2.6.2 compiled with identical 64-bit UCRT MinGW-w64 flags.
- [x] **3. Required SFML Modules Available:** `sfml-graphics`, `sfml-window`, and `sfml-system` resolved and linked.
- [x] **4. CMake Configuration Succeeds:** `cmake -S . -B build -G "Ninja"` outputs `Configuring done` and `Generating done` with exit code 0.
- [x] **5. Project Builds Successfully:** `cmake --build build` produces [build/sixty_four_squares.exe](file:///d:/64-Squares/build/sixty_four_squares.exe) without errors.
- [x] **6. Executable Starts Without Missing-Library Errors:** Static linking verified; zero runtime DLL dependencies required.
- [x] **7. SFML Window Opens and Closes Normally:** `sixty_four_squares.exe` starts, renders an 800×600 window titled `64 Squares`, and responds to close events.
- [ ] **8. Teammate 2 Reproduction:** *[Not verified — Pending independent checkout by Teammate 2]*
- [ ] **9. Teammate 3 Reproduction:** *[Not verified — Pending independent checkout by Teammate 3]*

---

> **Phase 0 Sign-Off Status:**  
> Teammate 1 (Host Developer) environment is **fully verified and functional**. Teammates 2 and 3 must execute the clean checkout procedure and verify Checklist items 8 and 9 before Phase 1 begins.
