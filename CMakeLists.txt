cmake_minimum_required(VERSION 3.28)

project(sixty_four_squares
    VERSION 0.1.0
    DESCRIPTION "64 Squares — C++ Chess Application using raylib"
    LANGUAGES C CXX
)

# ------------------------------------------------------------------------------
# 1. C++ Standard Specification
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# 2. Dependency Management: raylib 5.5
# ------------------------------------------------------------------------------
# Streamline raylib build by disabling unneeded examples and games
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable raylib examples" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "Disable raylib games" FORCE)

# Attempt to locate pre-installed raylib package; fallback to FetchContent from official repo release archive
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

# ------------------------------------------------------------------------------
# 3. Core Engine Library Target (Headless, independent of graphics)
# ------------------------------------------------------------------------------
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

# ------------------------------------------------------------------------------
# 4. Graphical Application Target
# ------------------------------------------------------------------------------
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

# ------------------------------------------------------------------------------
# 5. Automated Tests Target (Headless)
# ------------------------------------------------------------------------------
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

# Phase 3 Automated Tests Target (Headless)
add_executable(sixty_four_squares_phase3_tests
    tests/Phase3Tests.cpp
)

target_include_directories(sixty_four_squares_phase3_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(sixty_four_squares_phase3_tests PRIVATE
    sixty_four_squares_core
)

set_target_properties(sixty_four_squares_phase3_tests PROPERTIES
    OUTPUT_NAME "sixty_four_squares_phase3_tests"
)

add_test(NAME "64Squares_Phase3_Tests" COMMAND sixty_four_squares_phase3_tests)
set_tests_properties("64Squares_Phase3_Tests" PROPERTIES LABELS "64 Squares")

