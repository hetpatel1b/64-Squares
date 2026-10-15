cmake_minimum_required(VERSION 3.28)

project(SixtyFourSquares
    VERSION 0.1.0
    DESCRIPTION "64 Squares — C++ Chess Application"
    LANGUAGES CXX
)

# ------------------------------------------------------------------------------
# 1. C++ Standard Specification
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# 2. Dependency Management: SFML 2.6.2 (Graphics, Window, System)
# ------------------------------------------------------------------------------
# Default to static libraries on Windows to eliminate missing runtime DLL errors.
# Teammates can override with -DBUILD_SHARED_LIBS=ON if dynamic linking is preferred.
option(BUILD_SHARED_LIBS "Build libraries as shared (DLLs on Windows)" OFF)

# Attempt to locate pre-installed SFML package; fallback to FetchContent from official repo
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
# 3. Core Engine Library Target
# ------------------------------------------------------------------------------
add_library(sixty_four_squares STATIC
    src/ChessBoard.cpp
    src/ChessEngine.cpp
)

# Target-scoped include paths
target_include_directories(sixty_four_squares PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Target-scoped compiler features
target_compile_features(sixty_four_squares PUBLIC
    cxx_std_17
)

# ------------------------------------------------------------------------------
# 4. Development Application Target
# ------------------------------------------------------------------------------
add_executable(64Squares
    src/main.cpp
)

target_link_libraries(64Squares PRIVATE
    sixty_four_squares
)

set_target_properties(64Squares PROPERTIES
    OUTPUT_NAME "64Squares"
)

# ------------------------------------------------------------------------------
# 5. Automated Tests Target
# ------------------------------------------------------------------------------
enable_testing()

add_executable(sixty_four_squares_tests
    tests/Phase2Tests.cpp
)

target_link_libraries(sixty_four_squares_tests PRIVATE
    sixty_four_squares
)

set_target_properties(sixty_four_squares_tests PROPERTIES
    OUTPUT_NAME "sixty_four_squares_tests"
)

add_test(NAME "64Squares_Phase2_Tests" COMMAND sixty_four_squares_tests)
set_tests_properties("64Squares_Phase2_Tests" PROPERTIES LABELS "64 Squares")

# ------------------------------------------------------------------------------
# 6. Runtime DLL Deployment (Only active when BUILD_SHARED_LIBS is ON)
# ------------------------------------------------------------------------------
if(BUILD_SHARED_LIBS AND TARGET sfml-graphics)
    add_custom_command(TARGET 64Squares POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:64Squares>
            $<TARGET_FILE_DIR:64Squares>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime shared library DLLs to executable directory..."
    )
endif()
