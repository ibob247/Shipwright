cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

project(Ship VERSION 9.0.2 LANGUAGES C CXX)
include(CMake/soh-cvars.cmake)
include(CMake/lus-cvars.cmake)

find_package(OpenXR REQUIRED)

option(SUPPRESS_WARNINGS "Suppress warnings in LUS and src (decomp)" ON)
if(SUPPRESS_WARNINGS)
    MESSAGE("Suppressing warnings in LUS and src")
    if(MSVC)
        set(WARNING_OVERRIDE /w)
    else()
        set(WARNING_OVERRIDE -w)
    endif()
else()
    MESSAGE("Skipping warning suppression")
endif()

# Set startup project
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT soh)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(CMake/automate-vcpkg.cmake)
    set(VCPKG_TRIPLET x64-windows-static)
    set(VCPKG_TARGET_TRIPLET x64-windows-static)
    vcpkg_bootstrap()
    vcpkg_install_packages(zlib bzip2 libzip libpng sdl2 sdl2-net glew glfw3 nlohmann-json tinyxml2 spdlog)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(NOT CMAKE_VS_PLATFORM_NAME)
        set(CMAKE_VS_PLATFORM_NAME "x64")
    endif()
    message("${CMAKE_VS_PLATFORM_NAME} architecture in use")
    if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64" OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32"))
        message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    set(CMAKE_C_FLAGS_DEBUG "-g -ffast-math -DDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -ffast-math -DDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG")
else()
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_OBJCXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

include(CMake/Utils.cmake)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    get_linux_lsb_release_information()
    message(STATUS "Linux ${LSB_RELEASE_ID_SHORT} ${LSB_RELEASE_VERSION_SHORT} ${LSB_RELEASE_CODENAME_SHORT}")
else()
    message(STATUS ${CMAKE_SYSTEM_NAME})
endif()

include(CMake/GlobalSettingsInclude.cmake OPTIONAL)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(GFX_DEBUG_DISASSEMBLER ON)
set(GBI_UCODE F3DEX_GBI_2)
add_compile_definitions(CONTROLLERBUTTONS_T=uint32_t)

# Use unique build directories for subprojects
add_subdirectory(libultraship ${CMAKE_BINARY_DIR}/build-libultraship)
target_compile_options(libultraship PRIVATE "${WARNING_OVERRIDE}")
add_subdirectory(ZAPDTR/ZAPD ${CMAKE_BINARY_DIR}/build-zapd)
add_subdirectory(OTRExporter ${CMAKE_BINARY_DIR}/build-otrexporter)
add_subdirectory(soh)

# Link OpenXR to the soh target
if (TARGET soh)
    target_link_libraries(soh PRIVATE OpenXR::openxr_loader)
    target_include_directories(soh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/soh/src)
endif()
