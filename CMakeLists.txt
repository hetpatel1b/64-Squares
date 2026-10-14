cmake_minimum_required(VERSION 3.28)

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
# 3. Application Target
# ------------------------------------------------------------------------------
add_executable(KnightForge
    src/main.cpp
)

# Target-scoped include paths
target_include_directories(KnightForge PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Target-scoped compiler features
target_compile_features(KnightForge PRIVATE
    cxx_std_17
)

# Target-scoped SFML module linkage
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
# 4. Runtime DLL Deployment (Only active when BUILD_SHARED_LIBS is ON)
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
