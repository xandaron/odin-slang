cmake_minimum_required(VERSION 3.15)
project(slangc-wrapper VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set per-configuration output directories to avoid configuration subdirectories
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find required libraries
find_library(SLANG_LIB 
    NAMES slang
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/libs
    REQUIRED
)

find_library(SLANG_RT_LIB 
    NAMES slang-rt
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/libs
    REQUIRED
)

find_library(GFX_LIB 
    NAMES gfx
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/libs
    REQUIRED
)

# Create static library only
add_library(slangc STATIC
    slangc.cpp
    slangc.h
)

# Link libraries for static library
target_link_libraries(slangc
    ${SLANG_LIB}
    ${SLANG_RT_LIB}
    ${GFX_LIB}
)

# Set include directories
target_include_directories(slangc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compiler-specific options
if(MSVC)
    target_compile_options(slangc PRIVATE /W4)
    # Use static runtime library and avoid default library conflicts
    target_compile_options(slangc PRIVATE /MT)
    # Remove default libraries that cause conflicts
    target_link_options(slangc PRIVATE /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCRTD)
else()
    target_compile_options(slangc PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install targets
install(TARGETS slangc
    EXPORT slangc-targets
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

# Install headers
install(FILES slangc.h DESTINATION include)

# Install export file
install(EXPORT slangc-targets
    FILE slangc-targets.cmake
    NAMESPACE slangc::
    DESTINATION lib/cmake/slangc
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    slangc-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Create config file directly
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/slangc-config.cmake "
# slangc-config.cmake - CMake configuration for slangc library

get_filename_component(SLANGC_CMAKE_DIR \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)

# Include the targets file
include(\"\${SLANGC_CMAKE_DIR}/slangc-targets.cmake\")

# Create alias for easier usage
if(TARGET slangc AND NOT TARGET slangc::slangc)
    add_library(slangc::slangc ALIAS slangc)
endif()

# Set variables for backwards compatibility
set(SLANGC_LIBRARIES slangc::slangc)
set(SLANGC_INCLUDE_DIRS)
")

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/slangc-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/slangc-config-version.cmake
    DESTINATION lib/cmake/slangc
)
