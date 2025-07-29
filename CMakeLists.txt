cmake_minimum_required(VERSION 3.15)

project(SlangCWrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find or build Slang
find_path(SLANG_INCLUDE_DIR
    NAMES slang.h
    PATHS 
        ${CMAKE_CURRENT_SOURCE_DIR}/slang/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    NO_DEFAULT_PATH
)

find_library(SLANG_LIBRARY
    NAMES slang slang-rt
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/slang/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/slang/build/Release/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/slang/build/Debug/lib
    NO_DEFAULT_PATH
)

if(NOT SLANG_INCLUDE_DIR OR NOT SLANG_LIBRARY)
    message(STATUS "Slang not found, will build from source")
    
    # Add Slang as a subdirectory if available
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/slang/CMakeLists.txt)
        # Set options to minimize Slang build
        set(SLANG_ENABLE_TESTS OFF CACHE BOOL "")
        set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "")
        set(SLANG_ENABLE_GFX OFF CACHE BOOL "")
        set(SLANG_ENABLE_SLANG_GLSLANG OFF CACHE BOOL "")
        
        add_subdirectory(slang EXCLUDE_FROM_ALL)
        set(SLANG_TARGET slang)
    else()
        message(FATAL_ERROR "Slang library not found and source not available")
    endif()
else()
    message(STATUS "Found Slang: ${SLANG_LIBRARY}")
    message(STATUS "Slang include dir: ${SLANG_INCLUDE_DIR}")
    
    # Create imported target
    add_library(slang SHARED IMPORTED)
    set_target_properties(slang PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/slang/bin/slang.dll
        IMPORTED_IMPLIB ${SLANG_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
    )
    set(SLANG_TARGET slang)
endif()

# Create the C wrapper library (renamed to avoid conflict)
add_library(slang_wrapper SHARED
    slangc.cpp
    slangc.h
)

target_include_directories(slang_wrapper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(slang_wrapper PRIVATE ${SLANG_TARGET})

# Set output properties
set_target_properties(slang_wrapper PROPERTIES
    PUBLIC_HEADER slangc.h
    VERSION 1.0.0
    SOVERSION 1
)

# Installation
install(TARGETS slang_wrapper
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# Copy the header for binding generation
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/slangc.h ${CMAKE_CURRENT_BINARY_DIR}/slangc.h COPYONLY)

# Build example (optional)
option(BUILD_EXAMPLE "Build the example program" ON)
if(BUILD_EXAMPLE)
    add_executable(slangc_example example.c)
    set_target_properties(slangc_example PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(slangc_example slang_wrapper)
    target_include_directories(slangc_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    
    add_executable(slangc_spirv_example spirv_example.c)
    set_target_properties(slangc_spirv_example PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(slangc_spirv_example slang_wrapper)
    target_include_directories(slangc_spirv_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    
    add_executable(test_wrapper test_wrapper.c)
    set_target_properties(test_wrapper PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(test_wrapper slang_wrapper)
    target_include_directories(test_wrapper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
