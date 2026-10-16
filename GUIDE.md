# 64 Squares — CMake & raylib Windows Development Setup Guide

> **Document Type:** Team Standard Operating Procedure & Installation Manual  
> **Project:** 64 Squares (C++ Chess Application)  
> **Target Audience:** Engineering Team (3 Windows Developers using Visual Studio Code / Antigravity IDE)  
> **Scope:** Phase 2 — Environment Baseline, Toolchain Standardization, and raylib 5.5 Integration  

---

## Table of Contents

1. [A. Document Overview](#a-document-overview)
2. [B. Prerequisites](#b-prerequisites)
3. [C. Establish the Compiler Baseline](#c-establish-the-compiler-baseline)
4. [D. Install CMake](#d-install-cmake)
5. [E. Select and Configure raylib](#e-select-and-configure-raylib)
6. [F. Explain raylib Architecture & Modules](#f-explain-raylib-architecture--modules)
7. [G. Integrate raylib with CMake](#g-integrate-raylib-with-cmake)
8. [H. Build and Run the Project](#h-build-and-run-the-project)
9. [I. Runtime Dependencies](#i-runtime-dependencies)
10. [J. Troubleshooting Guide](#j-troubleshooting-guide)
11. [K. Three-Member Team Standardization](#k-three-member-team-standardization)
12. [L. Final Verification Checklist](#l-final-verification-checklist)

---

## A. Document Overview

### Purpose
This guide establishes a single, verified, and reproducible Windows development environment for the three-member team building **64 Squares**, a desktop chess application written in C++17 and rendered using **raylib 5.5**.

### Scope
This manual covers toolchain standardization, CMake configuration, raylib dependency resolution via `FetchContent`, building the core engine library and graphical application, running automated unit tests, and runtime verification.

### Expected End Result
Following this guide guarantees that on all three teammates' computers:
1. CMake configures the project cleanly without errors or warnings.
2. The C++17 compiler builds `libsixty_four_squares_core.a`, the test runner `sixty_four_squares_tests.exe`, and links `sixty_four_squares.exe` against raylib 5.5.
3. Automated unit tests execute and pass 100% via `ctest`.
4. The executable starts, displays an 800×800 interactive graphical chessboard window titled `64 Squares`, processes mouse clicks and keyboard events, and terminates cleanly with exit code 0.

### Conceptual Distinctions: Tool Roles
To prevent common configuration misunderstandings, understand the three distinct layers in this stack:
- **Installing CMake:** Installs a build-system generator executable (`cmake.exe`) that reads `CMakeLists.txt` and generates native build scripts for your compiler and generator (Ninja). CMake does not compile C++ code itself.
- **raylib Dependency Management:** Automatically handled via CMake `FetchContent`. Downloads official, pinned raylib 5.5 source archive and compiles it natively with your local compiler. Teammates do not need manual third-party package managers.
- **Integrating raylib into CMake:** Directs CMake where to locate raylib headers and static library targets (`raylib`), compile project source files against them, and link the final executable.

---

## B. Prerequisites

### Required Software

| Tool | Minimum Version | Recommended / Verified | Purpose |
| :--- | :--- | :--- | :--- |
| **Windows OS** | Windows 10 (64-bit) | Windows 11 (64-bit AMD64) | Host development environment |
| **C++ Compiler** | GCC 13.x (64-bit) | WinLibs MinGW-w64 GCC 16.2.0 (UCRT, POSIX threads, SEH) | C++17 compilation |
| **CMake** | 3.28 | CMake 4.4.4 (or 3.28+) | Build configuration generator |
| **Build Tool** | Ninja 1.11 | Ninja 1.13.2 | Fast parallel build execution |
| **Git** | Git 2.x | Git for Windows 2.40+ | Version control & dependency retrieval |
| **Graphics Library** | raylib 5.5 | raylib 5.5 (Static build via FetchContent) | Windowing, input handling, and 2D rendering |

### Recommended Visual Studio Code Extensions
- **C/C++ Extension Pack** (`ms-vscode.cpptools-extension-pack`)
- **CMake Tools** (`ms-vscode.cmake-tools`)

### The Rule of Binary Compatibility (CRITICAL)
In C++, **every compiled library must be 100% binary and ABI compatible with your compiler and runtime**.
- ⚠️ **Never mix 32-bit and 64-bit binaries.**
- ⚠️ **Never mix MSVC CRT binaries with MinGW GCC UCRT binaries.**
- ⚠️ **Always let CMake compile dependencies from source using your active local compiler.**

---

## C. Establish the Compiler Baseline

### Verification Commands
Open PowerShell and verify:
```powershell
g++ --version
where.exe g++
```

### Interpreting the Output
Expected output from the verified WinLibs MinGW-w64 compiler:
```text
g++ (MinGW-W64 x86_64-ucrt-posix-seh, built by Brecht Sanders, r1) 16.2.0
Target: x86_64-w64-mingw32
Thread model: posix
```

### Resolving the PATH Precedence Collision
If `where.exe g++` returns `C:\MinGW\bin\g++.exe` first, your system has an obsolete 32-bit GCC 6.3 installation that will fail on modern C++17.
Ensure your WinLibs directory appears first in `PATH`, or configure `.vscode/settings.json`:
```json
{
  "terminal.integrated.env.windows": {
    "PATH": "C:\\Users\\<user>\\AppData\\Local\\Microsoft\\WinGet\\Packages\\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\\mingw64\\bin;${env:PATH}"
  },
  "cmake.environment": {
    "PATH": "C:\\Users\\<user>\\AppData\\Local\\Microsoft\\WinGet\\Packages\\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\\mingw64\\bin;${env:PATH}"
  },
  "cmake.generator": "Ninja"
}
```

---

## D. Install CMake

Ensure CMake is installed and available in terminal:
```powershell
cmake --version
```
Expected: `cmake version 3.28` or newer (verified: CMake 4.4.4).

---

## E. Select and Configure raylib

### Version Decision: raylib 5.5
The team has standardized on **raylib 5.5** for the following technical reasons:

| Evaluation Factor | raylib 5.5 (Selected) | Notes |
| :--- | :--- | :--- |
| **API Simplicity** | Clean, lightweight, direct C/C++ API | Zero boilerplate window and input management |
| **Static Linking** | Compiles to self-contained `libraylib.a` | Zero external DLL headaches on Windows |
| **Reproducibility** | Official tagged release archive via FetchContent | Identical builds across all 3 developers |
| **Performance** | Native OpenGL 3.3 backend | Smooth 60 FPS rendering with minimal overhead |
| **Architectural Fit** | Easily decoupled via renderer abstraction | Keeps chess engine 100% headless and testable |

### Dependency Strategy: Automated CMake `FetchContent`
Rather than requiring teammates to install external package managers or download manual zip files:
- **64 Squares uses CMake's built-in `FetchContent` module.**
- CMake automatically downloads the official release archive `raylib-5.5.tar.gz` from GitHub:
  `https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz`
- Compiles raylib natively using your exact local compiler.
- Links statically into `sixty_four_squares.exe`.

---

## F. Explain raylib Architecture & Modules

raylib is modular and provides all required functionality:

```text
raylib
├── rcore      (Window creation, OpenGL context, input polling for mouse & keyboard)
├── rshapes    (2D primitive rendering: chessboard squares, outlines, badges)
├── rtextures  (Image loading, texture management, and sprite blitting)
├── rtext      (Font handling, text rendering, and string measurement)
├── raudio     (Audio device management and sound effects — optional)
└── rlgl       (Underlying OpenGL hardware abstraction layer)
```

### Decoupled Software Architecture:
1. **`sixty_four_squares_core`:** Static library containing `ChessBoard.cpp` and `ChessEngine.cpp`. Contains ZERO graphics dependencies. 100% portable and headless.
2. **`RaylibRenderer`:** Encapsulates all raylib calls (`InitWindow`, `BeginDrawing`, `DrawRectangle`, `DrawCircle`, `DrawText`, `EndDrawing`, `GetMousePosition`). Uses PIMPL to avoid header pollution.
3. **`BoardView`:** Presentation controller that translates `ChessEngine` state into visual draw calls and translates user mouse clicks into `engine.tryMove(from, to)`.
4. **`sixty_four_squares_tests`:** Headless test runner executing Phase 2 unit tests via `ctest`.

---

## G. Integrate raylib with CMake

Below is the verified `CMakeLists.txt` configured in the repository root:

```cmake
cmake_minimum_required(VERSION 3.28)

project(sixty_four_squares
    VERSION 0.1.0
    DESCRIPTION "64 Squares — C++ Chess Application using raylib"
    LANGUAGES C CXX
)

# 1. C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 2. raylib 5.5 Dependency
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable raylib examples" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "Disable raylib games" FORCE)

find_package(raylib 5.5 QUIET)
if(NOT raylib_FOUND)
    message(STATUS "raylib 5.5 not found locally. Fetching official raylib 5.5 release archive...")
    include(FetchContent)
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

# 3. Core Engine Library Target (Headless)
add_library(sixty_four_squares_core STATIC
    src/ChessBoard.cpp
    src/ChessEngine.cpp
)
target_include_directories(sixty_four_squares_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_compile_features(sixty_four_squares_core PUBLIC
    cxx_std_17
)

# 4. Graphical Application Target
add_executable(sixty_four_squares
    src/main.cpp
    src/BoardView.cpp
    src/RaylibRenderer.cpp
)
target_include_directories(sixty_four_squares PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_compile_features(sixty_four_squares PRIVATE
    cxx_std_17
)
target_link_libraries(sixty_four_squares PRIVATE
    sixty_four_squares_core
    raylib
)
set_target_properties(sixty_four_squares PROPERTIES
    OUTPUT_NAME "sixty_four_squares"
)

# 5. Automated Tests Target
enable_testing()
add_executable(sixty_four_squares_tests
    tests/Phase2Tests.cpp
)
target_include_directories(sixty_four_squares_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(sixty_four_squares_tests PRIVATE
    sixty_four_squares_core
)
set_target_properties(sixty_four_squares_tests PROPERTIES
    OUTPUT_NAME "sixty_four_squares_tests"
)
add_test(NAME "64Squares_Phase2_Tests" COMMAND sixty_four_squares_tests)
set_tests_properties("64Squares_Phase2_Tests" PROPERTIES LABELS "64 Squares")
```

---

## H. Build and Run the Project

Follow these exact steps from the repository root:

### Step 1: Open Terminal in Repository Root
```powershell
cd D:\64-Squares
```

### Step 2: Configure the Build Directory (Out-of-Source)
```powershell
cmake -S . -B build -G "Ninja"
```

### Step 3: Compile All Targets
```powershell
cmake --build build
```

### Step 4: Run Automated Tests
```powershell
ctest --test-dir build --output-on-failure
```
Or execute the test binary directly:
```powershell
./build/sixty_four_squares_tests.exe
```

### Step 5: Launch the 64 Squares Application
```powershell
./build/sixty_four_squares.exe
```

---

## I. Runtime Dependencies

- **Static Linking:** raylib is built as a static archive (`libraylib.a`) and linked directly into `sixty_four_squares.exe`.
- **System Libraries:** Standard Win32 system libraries (`-lopengl32 -lgdi32 -lwinmm`) are provided automatically by Windows OS.
- **No Third-Party DLLs Needed:** There are no external graphics DLLs required to run `sixty_four_squares.exe`.

---

## J. Troubleshooting Guide

| Problem Symptom | Likely Cause | Diagnostic Check | Resolution |
| :--- | :--- | :--- | :--- |
| **`cmake : The term 'cmake' is not recognized`** | CMake binary directory is missing from `PATH`. | `where.exe cmake` | Add CMake `bin` to User `PATH` or reopen terminal session. |
| **`g++ : The term 'g++' is not recognized`** | MinGW compiler is not in `PATH`. | `where.exe g++` | Add WinLibs `mingw64\bin` to `PATH`. |
| **CMake selects legacy 32-bit GCC 6.3** | `C:\MinGW\bin` appears before WinLibs in PATH. | `g++ -v` | Ensure WinLibs appears first in `PATH` or configure `.vscode/settings.json`. |
| **`ninja : The term 'ninja' is not recognized`** | Ninja build tool not reachable. | `where.exe ninja` | WinLibs distribution includes `ninja.exe` inside `mingw64\bin`. |
| **Build error: conflicting declaration 'Color'** | Including `raylib.h` and `Position.hpp` in same translation unit. | Compiler error message | Resolved by architecture: `RaylibRenderer` encapsulates raylib using PIMPL; `Position.hpp` is never exposed to `raylib.h`. |
| **Asset textures not loading** | Executable launched from non-standard directory. | Console log | `RaylibRenderer` uses automatic multi-path discovery (`assets/`, `../assets/`, and `GetApplicationDirectory()`), with crisp procedural fallback. |

---

## K. Three-Member Team Standardization

### Agreed Team Specification

| Parameter | Team Standard Value |
| :--- | :--- |
| **Project Identity** | 64 Squares (Executable: `sixty_four_squares.exe`) |
| **Host Operating System** | Windows 10/11 64-bit (`AMD64`) |
| **IDE & Extensions** | VS Code / Antigravity IDE + C/C++ Extension Pack + CMake Tools |
| **Compiler Distribution** | MinGW-w64 (WinLibs standalone distribution) |
| **Compiler Version & Architecture** | GCC 16.2.0 (`x86_64-w64-mingw32`, UCRT, POSIX threads, SEH) |
| **C++ Standard** | C++17 (`cxx_std_17`) |
| **Build System & Generator** | CMake >= 3.28 + Ninja |
| **Graphics Library** | raylib 5.5 (Official release archive, compiled statically via FetchContent) |
| **Standard Build Directory** | `build/` (Root out-of-source directory, excluded in `.gitignore`) |

---

## L. Final Verification Checklist

Every teammate must complete and verify each checkbox independently:

- [ ] **1. Compiler Executable Verified:** `where.exe g++` points to 64-bit MinGW-w64 (`x86_64-w64-mingw32`).
- [ ] **2. Compiler Version Verified:** `g++ --version` reports GCC 14.x or 16.x with UCRT and POSIX threads.
- [ ] **3. CMake Available in Terminal:** `cmake --version` returns version 3.28 or higher.
- [ ] **4. Build Generator Available:** `ninja --version` returns version 1.11 or higher.
- [ ] **5. Git Configured:** `git --version` returns Git 2.x.
- [ ] **6. Clean CMake Configuration:** `cmake -S . -B build -G "Ninja"` finishes with exit code 0.
- [ ] **7. Application Builds:** `cmake --build build` produces `build/sixty_four_squares.exe` with exit code 0.
- [ ] **8. Automated Tests Pass:** `ctest --test-dir build --output-on-failure` reports 100% tests passed.
- [ ] **9. Zero Runtime DLL Issues:** Executable runs without missing DLL errors.
- [ ] **10. Graphical Window Displays:** `sixty_four_squares.exe` launches an 800×800 window titled `64 Squares` with interactive chessboard and pieces.
- [ ] **11. Clean Termination:** Window closes cleanly when clicking the close button or pressing ESC.
