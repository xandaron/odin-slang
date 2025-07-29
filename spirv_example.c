#include "slangc.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Slang to SPIRV Compilation Example\n");
    printf("==================================\n\n");

    // Create global session
    printf("Creating global session...\n");
    SlangcGlobalSession* globalSession = NULL;
    SlangcResult result = slangc_createGlobalSession(&globalSession);
    if (result != SLANGC_OK) {
        printf("ERROR: Failed to create global session (error code: %d)\n", result);
        return 1;
    }
    printf("✓ Global session created successfully\n");

    // Create compilation session with SPIRV target
    printf("Creating compilation session with SPIRV target...\n");
    SlangcSessionDesc sessionDesc = {};
    SlangcCompileTarget targets[] = { SLANGC_TARGET_SPIRV };
    sessionDesc.targets = targets;
    sessionDesc.targetCount = 1;

    SlangcSession* session = NULL;
    result = slangc_createSession(globalSession, &sessionDesc, &session);
    if (result != SLANGC_OK) {
        printf("ERROR: Failed to create session (error code: %d)\n", result);
        slangc_releaseGlobalSession(globalSession);
        return 1;
    }
    printf("✓ Compilation session created with SPIRV target\n");

    // Load a simple compute shader from source
    printf("Loading compute shader from source...\n");
    const char* computeShaderSource = 
        "RWStructuredBuffer<float> outputBuffer;\n"
        "\n"
        "[numthreads(64, 1, 1)]\n"
        "void computeMain(uint3 id : SV_DispatchThreadID) {\n"
        "    uint index = id.x;\n"
        "    outputBuffer[index] = float(index) * 2.0;\n"
        "}\n";

    SlangcModule* module = NULL;
    SlangcBlob* moduleDiagnostics = NULL;
    result = slangc_loadModuleFromSource(
        session,
        "ComputeShader",
        "compute.slang",
        computeShaderSource,
        0, // let it calculate length
        &module,
        &moduleDiagnostics
    );

    if (result != SLANGC_OK) {
        printf("ERROR: Failed to load module (error code: %d)\n", result);
        if (moduleDiagnostics) {
            printf("Module diagnostics: %s\n", (const char*)slangc_getBlobData(moduleDiagnostics));
            slangc_releaseBlob(moduleDiagnostics);
        }
        slangc_releaseSession(session);
        slangc_releaseGlobalSession(globalSession);
        return 1;
    }
    printf("✓ Module loaded successfully\n");

    if (moduleDiagnostics) {
        size_t diagSize = slangc_getBlobSize(moduleDiagnostics);
        if (diagSize > 0) {
            printf("Module load diagnostics: %s\n", (const char*)slangc_getBlobData(moduleDiagnostics));
        }
        slangc_releaseBlob(moduleDiagnostics);
    }

    // Find the entry point
    printf("Finding compute shader entry point...\n");
    SlangcEntryPoint* entryPoint = NULL;
    SlangcBlob* entryPointDiagnostics = NULL;
    result = slangc_findEntryPoint(
        module,
        "computeMain",
        SLANGC_STAGE_COMPUTE,
        &entryPoint,
        &entryPointDiagnostics
    );

    if (result != SLANGC_OK) {
        printf("ERROR: Failed to find entry point (error code: %d)\n", result);
        if (entryPointDiagnostics) {
            printf("Entry point diagnostics: %s\n", (const char*)slangc_getBlobData(entryPointDiagnostics));
            slangc_releaseBlob(entryPointDiagnostics);
        }
        slangc_releaseModule(module);
        slangc_releaseSession(session);
        slangc_releaseGlobalSession(globalSession);
        return 1;
    }
    printf("✓ Entry point found successfully\n");

    if (entryPointDiagnostics) {
        size_t diagSize = slangc_getBlobSize(entryPointDiagnostics);
        if (diagSize > 0) {
            printf("Entry point diagnostics: %s\n", (const char*)slangc_getBlobData(entryPointDiagnostics));
        }
        slangc_releaseBlob(entryPointDiagnostics);
    }

    // Create a component type from the module and entry point
    printf("Creating component type from module and entry point...\n");
    SlangcComponentType* componentType = NULL;
    SlangcBlob* componentDiagnostics = NULL;
    result = slangc_createModuleComponentType(
        session,
        module,
        entryPoint,
        &componentType,
        &componentDiagnostics
    );

    if (result != SLANGC_OK) {
        printf("ERROR: Failed to create component type (error code: %d)\n", result);
        if (componentDiagnostics) {
            printf("Component type diagnostics: %s\n", (const char*)slangc_getBlobData(componentDiagnostics));
            slangc_releaseBlob(componentDiagnostics);
        }
        slangc_releaseEntryPoint(entryPoint);
        slangc_releaseModule(module);
        slangc_releaseSession(session);
        slangc_releaseGlobalSession(globalSession);
        return 1;
    }
    printf("✓ Component type created successfully\n");

    if (componentDiagnostics) {
        size_t diagSize = slangc_getBlobSize(componentDiagnostics);
        if (diagSize > 0) {
            printf("Component type diagnostics: %s\n", (const char*)slangc_getBlobData(componentDiagnostics));
        }
        slangc_releaseBlob(componentDiagnostics);
    }

    // Get the number of entry points
    int32_t entryPointCount = slangc_getEntryPointCount(componentType);
    printf("✓ Component type has %d entry point(s)\n", entryPointCount);

    // Compile to SPIRV
    printf("Compiling to SPIRV...\n");
    SlangcBlob* sprivCode = NULL;
    SlangcBlob* compileDiagnostics = NULL;
    result = slangc_getEntryPointCode(
        componentType,
        0, // first entry point
        0, // first target (SPIRV)
        &sprivCode,
        &compileDiagnostics
    );

    if (result != SLANGC_OK) {
        printf("ERROR: Failed to compile to SPIRV (error code: %d)\n", result);
        if (compileDiagnostics) {
            printf("Compilation diagnostics: %s\n", (const char*)slangc_getBlobData(compileDiagnostics));
            slangc_releaseBlob(compileDiagnostics);
        }
        slangc_releaseComponentType(componentType);
        slangc_releaseEntryPoint(entryPoint);
        slangc_releaseModule(module);
        slangc_releaseSession(session);
        slangc_releaseGlobalSession(globalSession);
        return 1;
    }

    if (sprivCode) {
        size_t sprivSize = slangc_getBlobSize(sprivCode);
        const uint8_t* sprivData = (const uint8_t*)slangc_getBlobData(sprivCode);
        
        printf("✓ SPIRV compilation successful!\n");
        printf("  SPIRV size: %zu bytes\n", sprivSize);
        printf("  SPIRV magic number: 0x%08X\n", 
               sprivSize >= 4 ? *(const uint32_t*)sprivData : 0);
        
        // Try to save to file
        FILE* spvFile = fopen("compute.spv", "wb");
        if (spvFile) {
            fwrite(sprivData, 1, sprivSize, spvFile);
            fclose(spvFile);
            printf("  SPIRV saved to compute.spv\n");
        } else {
            printf("  WARNING: Could not save SPIRV to file\n");
        }
        
        slangc_releaseBlob(sprivCode);
    } else {
        printf("WARNING: No SPIRV code generated\n");
    }

    if (compileDiagnostics) {
        size_t diagSize = slangc_getBlobSize(compileDiagnostics);
        if (diagSize > 0) {
            printf("Compilation diagnostics: %s\n", (const char*)slangc_getBlobData(compileDiagnostics));
        }
        slangc_releaseBlob(compileDiagnostics);
    }

    // Clean up
    printf("Cleaning up...\n");
    slangc_releaseComponentType(componentType);
    slangc_releaseEntryPoint(entryPoint);
    slangc_releaseModule(module);
    slangc_releaseSession(session);
    slangc_releaseGlobalSession(globalSession);
    slangc_shutdown();

    printf("✓ SPIRV compilation example completed successfully!\n");
    return 0;
}
