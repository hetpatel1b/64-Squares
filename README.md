# 64 Squares

64 Squares is a modern desktop chess application built in C++ utilizing the SFML graphics and windowing library. Developed collaboratively by a three-member engineering team, the project emphasizes modular architecture, robust game state management, and smooth interactive gameplay.

---

## Project Status

- **Current Phase:** Phase 0 — Development Foundation (Step 3: SFML + CMake Integration)
- **Active Focus:** Standardizing the team development environment, project directory structure, and build toolchain.
- **Implementation Note:** Game logic, chess rules, and graphical windowing features have not yet been implemented.

---

## Standardized Team Toolchain

All three teammates must adhere to this verified toolchain specification to prevent binary and ABI incompatibilities:

| Component | Standardized Specification |
| :--- | :--- |
| **Operating System** | Windows 10/11 (64-bit AMD64) |
| **Editor / IDE** | Visual Studio Code (with C/C++ Extension Pack & CMake Tools) |
| **Compiler Distribution** | MinGW-w64 (WinLibs standalone distribution) |
| **Compiler Version** | GCC 16.2.0 (`x86_64-w64-mingw32`, UCRT runtime, POSIX threads, SEH exceptions) |
| **Build System** | CMake >= 3.28 (Verified: CMake 4.4.4) |
| **Build Generator** | Ninja 1.13.2 (or MinGW Makefiles) |
| **Version Control** | Git for Windows (64-bit) |
| **Language Standard** | C++17 (to be formally configured with SFML in Step 3) |

> **Important:** All teammates must follow the exact same toolchain and build configuration. Mixing 32-bit compilers (such as legacy MinGW.org GCC 6.3) or MSVC toolchains with MinGW-w64 SFML binaries is strictly prohibited.

---

## Build Instructions

The project uses CMake with Ninja. CMake automatically resolves and compiles SFML 2.6.2 via `FetchContent`:

```powershell
# 1. Configure the project build tree using Ninja
cmake -S . -B build -G "Ninja"

# 2. Compile sixty_four_squares
cmake --build build

# 3. Execute the binary
./build/sixty_four_squares.exe
```

> **Note on Static Linking:** By default, `BUILD_SHARED_LIBS` is set to `OFF`, producing a self-contained executable with zero external SFML runtime DLL dependencies. If shared libraries are desired, pass `-DBUILD_SHARED_LIBS=ON` during configuration (CMake will automatically deploy the required DLLs to the build folder).

---

## Repository Structure

```text
64-Squares/
├── assets/
│   ├── pieces/         # Chess piece sprites and graphics
│   └── fonts/          # UI typography assets
├── include/            # Public header files (.hpp)
├── src/                # Implementation files (.cpp)
│   └── main.cpp        # Application entry point skeleton
├── tests/              # Unit and integration test suites
├── saves/              # Local game save files (ignored by Git)
├── CMakeLists.txt      # Root CMake configuration
├── README.md           # Project documentation
├── GUIDE.md            # Comprehensive installation and setup guide
└── .gitignore          # Build, binary, and OS ignore patterns
```

---

## Team Git Workflow

To maintain clean and traceable collaboration among all three teammates:

1. **Clone the repository:**
   ```powershell
   git clone https://github.com/hetpatel1b/64-Squares.git
   cd 64-Squares
   ```
2. **Create a feature branch:**
   ```powershell
   git checkout -b feature/<branch-name>
   ```
3. **Commit focused, incremental changes:**
   ```powershell
   git add <files>
   git commit -m "feat(<scope>): concise commit description"
   ```
4. **Push the branch to the remote repository:**
   ```powershell
   git push -u origin feature/<branch-name>
   ```
5. **Open a Pull Request:** Submit a PR on GitHub for peer review before merging into `main`.
6. **Merge after review:** Ensure local tests pass and approvals are granted before merging.
