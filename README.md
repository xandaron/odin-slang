# Slang C Wrapper

This project provides a C wrapper around the Slang shader compiler C++ API, making it easier to create bindings for other languages like Odin.

## Overview

The Slang shader compiler provides a C++ API that uses COM-style interfaces. While this is already somewhat C-compatible, this wrapper provides a cleaner C interface with:

- Opaque handle types instead of C++ classes
- Simple C function calls instead of virtual method calls
- Proper memory management functions
- Error handling through return codes
- Simplified parameter passing

## Files

- `slangc.h` - C header file with the wrapper API
- `slangc.cpp` - C++ implementation that wraps the Slang API
- `CMakeLists.txt` - Build configuration
- `build.bat` - Windows build script
- `bindgen.sjson` - Configuration for generating Odin bindings

## Building

### Prerequisites

1. Slang library built and available
2. CMake 3.15 or later
3. Visual Studio 2022 or compatible C++ compiler

### Build Steps

1. Clone/download the Slang repository to the `slang/` subdirectory, or ensure you have Slang libraries available
2. Run the build script:
   ```cmd
   build.bat
   ```
   
   Or manually with CMake:
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   cmake --build . --config Release
   ```

## Generating Odin Bindings

After building the C wrapper, you can generate Odin bindings using your bindgen tool:

```cmd
bindgen -config bindgen.sjson
```

This will generate Odin bindings in the `odin-slang/` directory.

## API Overview

The C wrapper provides these main categories of functions:

### Global Session Management
- `slangc_createGlobalSession()` - Create a global session
- `slangc_releaseGlobalSession()` - Release a global session  
- `slangc_findProfile()` - Find a compilation profile by name

### Session Management
- `slangc_createSession()` - Create a compilation session
- `slangc_releaseSession()` - Release a session

### Module Loading
- `slangc_loadModule()` - Load a module by name
- `slangc_loadModuleFromSource()` - Load a module from source code
- `slangc_releaseModule()` - Release a module

### Compilation (Modern API)
- `slangc_createCompositeComponentType()` - Combine multiple components
- `slangc_compileComponentType()` - Compile a component to target code

### Compilation (Legacy API)
- `slangc_createCompileRequest()` - Create a compile request
- `slangc_addTranslationUnit()` - Add source code to compile
- `slangc_addEntryPoint()` - Add a shader entry point
- `slangc_setTarget()` - Set compilation target
- `slangc_compile()` - Perform compilation
- `slangc_getEntryPointCode()` - Get compiled code
- `slangc_getDiagnosticOutput()` - Get compilation diagnostics

### Blob Management
- `slangc_getBlobData()` - Get data from a blob
- `slangc_getBlobSize()` - Get blob size
- `slangc_releaseBlob()` - Release a blob

### Utilities
- `slangc_getVersionString()` - Get version information
- `slangc_shutdown()` - Shutdown the Slang system
- `slangc_getLastErrorMessage()` - Get last error message

## Example Usage (C)

```c
#include "slangc.h"
#include <stdio.h>

int main() {
    // Create global session
    SlangcGlobalSession* globalSession = NULL;
    if (slangc_createGlobalSession(&globalSession) != SLANGC_OK) {
        printf("Failed to create global session\n");
        return 1;
    }

    // Create compilation session
    SlangcSession* session = NULL;
    if (slangc_createSession(globalSession, NULL, &session) != SLANGC_OK) {
        printf("Failed to create session\n");
        slangc_releaseGlobalSession(globalSession);
        return 1;
    }

    // Create compile request
    SlangcCompileRequest* request = NULL;
    if (slangc_createCompileRequest(session, &request) != SLANGC_OK) {
        printf("Failed to create compile request\n");
        slangc_releaseSession(session);
        slangc_releaseGlobalSession(globalSession);
        return 1;
    }

    // Add source code
    const char* source = "float4 main() : SV_Target { return float4(1,0,0,1); }";
    int unitIndex = slangc_addTranslationUnit(request, SLANGC_SOURCE_LANGUAGE_HLSL, "test.hlsl", source);
    
    // Add entry point
    int entryIndex = slangc_addEntryPoint(request, unitIndex, "main", SLANGC_STAGE_FRAGMENT);
    
    // Set target
    slangc_setTarget(request, SLANGC_TARGET_HLSL);
    
    // Compile
    if (slangc_compile(request) == SLANGC_OK) {
        SlangcBlob* code = NULL;
        if (slangc_getEntryPointCode(request, entryIndex, &code) == SLANGC_OK) {
            printf("Compiled successfully!\n");
            printf("Code size: %zu bytes\n", slangc_getBlobSize(code));
            slangc_releaseBlob(code);
        }
    } else {
        SlangcBlob* diagnostics = NULL;
        if (slangc_getDiagnosticOutput(request, &diagnostics) == SLANGC_OK) {
            printf("Compilation failed:\n%s\n", (char*)slangc_getBlobData(diagnostics));
            slangc_releaseBlob(diagnostics);
        }
    }

    // Cleanup
    slangc_releaseCompileRequest(request);
    slangc_releaseSession(session);
    slangc_releaseGlobalSession(globalSession);
    slangc_shutdown();

    return 0;
}
```

## Error Handling

All functions return `SlangcResult` codes:
- `SLANGC_OK` (0) - Success
- `SLANGC_FAIL` (-1) - General failure
- `SLANGC_E_NOT_IMPLEMENTED` (-2) - Feature not implemented
- `SLANGC_E_INVALID_ARG` (-3) - Invalid argument
- `SLANGC_E_OUT_OF_MEMORY` (-4) - Out of memory
- `SLANGC_E_BUFFER_TOO_SMALL` (-5) - Buffer too small

## Memory Management

The wrapper uses a simple ownership model:
- All `create*` functions return objects that must be released with corresponding `release*` functions
- Blob objects returned from compilation must be released with `slangc_releaseBlob()`
- Always call `slangc_shutdown()` after releasing all objects to clean up global state

## License

This wrapper follows the same license as the Slang project it wraps.
