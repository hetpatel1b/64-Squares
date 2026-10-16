# 64 Squares

64 Squares is a modern desktop chess application built in C++ utilizing the **raylib** graphics and windowing library. Developed collaboratively by a three-member engineering team, the project emphasizes modular architecture, robust game state management, and smooth interactive gameplay.

---

## Project Status

- **Current Phase:** Phase 2 — Core Engine & raylib Graphics Integration
- **Active Focus:** Interactive 2D graphical chessboard, piece move execution, legal move validation, and automated test suite.
- **Architecture Highlights:** Strict separation of concerns between the headless chess engine (`sixty_four_squares_core`), graphics abstraction layer (`RaylibRenderer`), board presentation controller (`BoardView`), and unit test suite (`sixty_four_squares_tests`).

---

## Standardized Team Toolchain

All three teammates must adhere to this verified toolchain specification to prevent binary and ABI incompatibilities:

| Component | Standardized Specification |
| :--- | :--- |
| **Operating System** | Windows 10/11 (64-bit AMD64) |
| **Editor / IDE** | Visual Studio Code / Antigravity IDE (with C/C++ Extension Pack & CMake Tools) |
| **Compiler Distribution** | MinGW-w64 (WinLibs standalone distribution) |
| **Compiler Version** | GCC 16.2.0 (`x86_64-w64-mingw32`, UCRT runtime, POSIX threads, SEH exceptions) |
| **Build System** | CMake >= 3.28 (Verified: CMake 4.4.4) |
| **Build Generator** | Ninja 1.13.2 (or MinGW Makefiles) |
| **Graphics Library** | raylib 5.5 (Static build via automated CMake `FetchContent`) |
| **Language Standard** | C++17 |

> **Important:** All teammates must follow the exact same toolchain and build configuration. Mixing 32-bit compilers (such as legacy MinGW.org GCC 6.3) with 64-bit raylib libraries is strictly prohibited.

---

## Build Instructions

The project uses CMake with Ninja. CMake automatically resolves, downloads, and compiles raylib 5.5 via `FetchContent`:

```powershell
# 1. Configure the project build tree using Ninja
cmake -S . -B build -G "Ninja"

# 2. Compile sixty_four_squares executable and test suite
cmake --build build

# 3. Execute the 64 Squares desktop application
./build/sixty_four_squares.exe

# 4. Run automated test suite
ctest --test-dir build --output-on-failure
```

> **Note on Static Linking:** raylib is compiled as a static library (`libraylib.a`), producing a self-contained executable with zero external runtime DLL dependencies on Windows.

---

## Controls & Gameplay

- **Left Mouse Click:** Select a piece of the active player's color, then click a target square to execute a move.
- **Right Mouse Click / ESC:** Cancel piece selection.
- **R Key:** Reset the chessboard to the initial starting position.

---

## Repository Structure

```text
64-Squares/
├── assets/
│   ├── pieces/         # Chess piece sprites and graphics (.png)
│   └── fonts/          # UI typography assets
├── include/            # Public header files (.hpp)
│   ├── Bishop.hpp
│   ├── BoardView.hpp
│   ├── ChessBoard.hpp
│   ├── ChessEngine.hpp
│   ├── King.hpp
│   ├── Knight.hpp
│   ├── Pawn.hpp
│   ├── Piece.hpp
│   ├── Position.hpp
│   ├── Queen.hpp
│   ├── RaylibRenderer.hpp
│   └── Rook.hpp
├── src/                # Implementation files (.cpp)
│   ├── BoardView.cpp
│   ├── ChessBoard.cpp
│   ├── ChessEngine.cpp
│   ├── main.cpp
│   └── RaylibRenderer.cpp
├── tests/              # Unit and integration test suites
│   └── Phase2Tests.cpp
├── saves/              # Local game save files (ignored by Git)
├── CMakeLists.txt      # Root CMake configuration
├── README.md           # Project overview and quick start
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
