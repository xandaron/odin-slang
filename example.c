#include "slangc.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Slang C Wrapper Example\n");
    printf("======================\n\n");

    // Create global session
    printf("Creating global session...\n");
    SlangcGlobalSession* globalSession = NULL;
    SlangcResult result = slangc_createGlobalSession(&globalSession);
    if (result != SLANGC_OK) {
        printf("ERROR: Failed to create global session (error code: %d)\n", result);
        return 1;
    }
    printf("✓ Global session created successfully\n");

    // Create compilation session
    printf("Creating compilation session...\n");
    SlangcSession* session = NULL;
    result = slangc_createSession(globalSession, NULL, &session);
    if (result != SLANGC_OK) {
        printf("ERROR: Failed to create session (error code: %d)\n", result);
        slangc_releaseGlobalSession(globalSession);
        return 1;
    }
    printf("✓ Compilation session created successfully\n");

    // Try to load a simple module from source
    printf("Loading module from source...\n");
    const char* moduleSource = 
        "// Simple Slang module\n"
        "float4 simpleVertex(float3 pos : POSITION) : SV_Position {\n"
        "    return float4(pos, 1.0);\n"
        "}\n\n"
        "float4 simpleFragment() : SV_Target {\n"
        "    return float4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    SlangcModule* module = NULL;
    SlangcBlob* diagnostics = NULL;
    result = slangc_loadModuleFromSource(
        session,
        "SimpleShader",
        "simple.slang",
        moduleSource,
        0, // let it calculate length
        &module,
        &diagnostics
    );

    if (result != SLANGC_OK) {
        printf("ERROR: Failed to load module (error code: %d)\n", result);
        if (diagnostics) {
            printf("Diagnostics: %s\n", (const char*)slangc_getBlobData(diagnostics));
            slangc_releaseBlob(diagnostics);
        }
        slangc_releaseSession(session);
        slangc_releaseGlobalSession(globalSession);
        return 1;
    }
    printf("✓ Module loaded successfully\n");

    if (diagnostics) {
        size_t diagSize = slangc_getBlobSize(diagnostics);
        if (diagSize > 0) {
            printf("Module diagnostics: %s\n", (const char*)slangc_getBlobData(diagnostics));
        }
        slangc_releaseBlob(diagnostics);
    }

    // Test the legacy compile request API (should return not implemented)
    printf("Testing legacy compile request API...\n");
    SlangcCompileRequest* request = NULL;
    result = slangc_createCompileRequest(session, &request);
    if (result == SLANGC_E_NOT_IMPLEMENTED) {
        printf("✓ Legacy API correctly returns not implemented\n");
    } else {
        printf("WARNING: Expected SLANGC_E_NOT_IMPLEMENTED but got %d\n", result);
    }

    // Find some profiles
    printf("Looking up shader profiles...\n");
    SlangcProfileID vs5Profile = slangc_findProfile(globalSession, "vs_5_0");
    SlangcProfileID ps5Profile = slangc_findProfile(globalSession, "ps_5_0");
    
    if (vs5Profile != 0) {
        printf("✓ Found vertex shader 5.0 profile (ID: %u)\n", vs5Profile);
    } else {
        printf("? Could not find vs_5_0 profile\n");
    }
    
    if (ps5Profile != 0) {
        printf("✓ Found pixel shader 5.0 profile (ID: %u)\n", ps5Profile);
    } else {
        printf("? Could not find ps_5_0 profile\n");
    }

    // Clean up
    printf("Cleaning up...\n");
    if (module) {
        slangc_releaseModule(module);
    }
    slangc_releaseSession(session);
    slangc_releaseGlobalSession(globalSession);
    slangc_shutdown();

    printf("✓ Example completed successfully!\n");
    printf("\nNote: This example demonstrates the basic C wrapper functionality.\n");
    printf("For actual shader compilation, you would use the component type API\n");
    printf("to compile loaded modules to specific targets.\n");
    
    return 0;
}
