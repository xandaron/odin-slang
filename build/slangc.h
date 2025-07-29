#ifndef SLANGC_H
#define SLANGC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Forward declarations for opaque handles
typedef struct SlangcGlobalSession SlangcGlobalSession;
typedef struct SlangcSession SlangcSession;
typedef struct SlangcModule SlangcModule;
typedef struct SlangcEntryPoint SlangcEntryPoint;
typedef struct SlangcComponentType SlangcComponentType;
typedef struct SlangcBlob SlangcBlob;
typedef struct SlangcCompileRequest SlangcCompileRequest;

// Result codes
typedef int32_t SlangcResult;
#define SLANGC_OK 0
#define SLANGC_FAIL -1
#define SLANGC_E_NOT_IMPLEMENTED -2
#define SLANGC_E_INVALID_ARG -3
#define SLANGC_E_OUT_OF_MEMORY -4
#define SLANGC_E_BUFFER_TOO_SMALL -5

// API version
#define SLANGC_API_VERSION 1

// Source languages
typedef enum SlangcSourceLanguage {
    SLANGC_SOURCE_LANGUAGE_UNKNOWN = 0,
    SLANGC_SOURCE_LANGUAGE_SLANG,
    SLANGC_SOURCE_LANGUAGE_HLSL,
    SLANGC_SOURCE_LANGUAGE_GLSL,
    SLANGC_SOURCE_LANGUAGE_C,
    SLANGC_SOURCE_LANGUAGE_CPP
} SlangcSourceLanguage;

// Compilation targets
typedef enum SlangcCompileTarget {
    SLANGC_TARGET_UNKNOWN = 0,
    SLANGC_TARGET_NONE,
    SLANGC_TARGET_HLSL,
    SLANGC_TARGET_GLSL,
    SLANGC_TARGET_SPIRV,
    SLANGC_TARGET_SPIRV_ASM,
    SLANGC_TARGET_DXBC,
    SLANGC_TARGET_DXBC_ASM,
    SLANGC_TARGET_DXIL,
    SLANGC_TARGET_DXIL_ASM,
    SLANGC_TARGET_C_SOURCE,
    SLANGC_TARGET_CPP_SOURCE,
    SLANGC_TARGET_CUDA_SOURCE,
    SLANGC_TARGET_PTX,
    SLANGC_TARGET_CUBIN,
    SLANGC_TARGET_METAL,
    SLANGC_TARGET_METAL_LIB,
    SLANGC_TARGET_HOST_CALLABLE,
    SLANGC_TARGET_SHADER_SHARED_LIBRARY,
    SLANGC_TARGET_SHADER_HOST_CALLABLE,
    SLANGC_TARGET_WGSL
} SlangcCompileTarget;

// Stage types
typedef enum SlangcStage {
    SLANGC_STAGE_NONE = 0,
    SLANGC_STAGE_VERTEX,
    SLANGC_STAGE_HULL,
    SLANGC_STAGE_DOMAIN,
    SLANGC_STAGE_GEOMETRY,
    SLANGC_STAGE_FRAGMENT,
    SLANGC_STAGE_COMPUTE,
    SLANGC_STAGE_RAY_GENERATION,
    SLANGC_STAGE_INTERSECTION,
    SLANGC_STAGE_ANY_HIT,
    SLANGC_STAGE_CLOSEST_HIT,
    SLANGC_STAGE_MISS,
    SLANGC_STAGE_CALLABLE,
    SLANGC_STAGE_MESH,
    SLANGC_STAGE_AMPLIFICATION
} SlangcStage;

// Profile IDs (opaque)
typedef uint32_t SlangcProfileID;

// Session description
typedef struct SlangcSessionDesc {
    SlangcCompileTarget* targets;
    int32_t targetCount;
    const char** searchPaths;
    int32_t searchPathCount;
    const char** preprocessorMacros;
    int32_t preprocessorMacroCount;
} SlangcSessionDesc;

// Global session description
typedef struct SlangcGlobalSessionDesc {
    uint32_t apiVersion;
    const char** searchPaths;
    int32_t searchPathCount;
    bool enableGLSL;
} SlangcGlobalSessionDesc;

//
// Global Session Management
//

/** Create a global session with default settings.
 * @param outGlobalSession Pointer to receive the created global session
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_createGlobalSession(SlangcGlobalSession** outGlobalSession);

/** Create a global session with custom settings.
 * @param desc Description of the global session configuration
 * @param outGlobalSession Pointer to receive the created global session
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_createGlobalSessionWithDesc(
    const SlangcGlobalSessionDesc* desc,
    SlangcGlobalSession** outGlobalSession);

/** Release a global session and free its resources.
 * @param globalSession The global session to release
 */
void slangc_releaseGlobalSession(SlangcGlobalSession* globalSession);

/** Find a profile by name.
 * @param globalSession The global session
 * @param name The profile name (e.g., "vs_5_0", "ps_5_0", "cs_5_0")
 * @return The profile ID, or 0 if not found
 */
SlangcProfileID slangc_findProfile(SlangcGlobalSession* globalSession, const char* name);

//
// Session Management  
//

/** Create a compilation session.
 * @param globalSession The global session
 * @param desc Session description (can be NULL for defaults)
 * @param outSession Pointer to receive the created session
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_createSession(
    SlangcGlobalSession* globalSession,
    const SlangcSessionDesc* desc,
    SlangcSession** outSession);

/** Release a session and free its resources.
 * @param session The session to release
 */
void slangc_releaseSession(SlangcSession* session);

//
// Module Loading
//

/** Load a module by name (for import statements).
 * @param session The compilation session
 * @param moduleName The name of the module to load
 * @param outModule Pointer to receive the loaded module
 * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_loadModule(
    SlangcSession* session,
    const char* moduleName,
    SlangcModule** outModule,
    SlangcBlob** outDiagnostics);

/** Load a module from source code.
 * @param session The compilation session
 * @param moduleName The name to give the module
 * @param path The file path (for error reporting)
 * @param sourceText The source code text
 * @param sourceSize The size of the source code (or 0 if null-terminated)
 * @param outModule Pointer to receive the loaded module
 * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_loadModuleFromSource(
    SlangcSession* session,
    const char* moduleName,
    const char* path,
    const char* sourceText,
    size_t sourceSize,
    SlangcModule** outModule,
    SlangcBlob** outDiagnostics);

/** Release a module.
 * @param module The module to release
 */
void slangc_releaseModule(SlangcModule* module);

//
// Component Type Management
//

/** Create a composite component type from multiple components.
 * @param session The compilation session
 * @param componentTypes Array of component types to combine
 * @param componentTypeCount Number of component types
 * @param outComposite Pointer to receive the composite component type
 * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_createCompositeComponentType(
    SlangcSession* session,
    SlangcComponentType* const* componentTypes,
    int32_t componentTypeCount,
    SlangcComponentType** outComposite,
    SlangcBlob** outDiagnostics);

/** Find and check an entry point in a module.
 * @param module The loaded module
 * @param entryPointName The name of the entry point function
 * @param stage The shader stage for the entry point
 * @param outEntryPoint Pointer to receive the entry point
 * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_findEntryPoint(
    SlangcModule* module,
    const char* entryPointName,
    SlangcStage stage,
    SlangcEntryPoint** outEntryPoint,
    SlangcBlob** outDiagnostics);

/** Release an entry point.
 * @param entryPoint The entry point to release
 */
void slangc_releaseEntryPoint(SlangcEntryPoint* entryPoint);

/** Create a component type from a module and entry point.
 * @param session The compilation session
 * @param module The module containing the entry point
 * @param entryPoint The entry point
 * @param outComponentType Pointer to receive the component type
 * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_createModuleComponentType(
    SlangcSession* session,
    SlangcModule* module,
    SlangcEntryPoint* entryPoint,
    SlangcComponentType** outComponentType,
    SlangcBlob** outDiagnostics);

/** Get entry point code for a specific entry point and target.
 * @param componentType The component type to compile
 * @param entryPointIndex The entry point index (0-based)
 * @param targetIndex The target index (0-based, from session targets)
 * @param outCode Pointer to receive the compiled code
 * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_getEntryPointCode(
    SlangcComponentType* componentType,
    int32_t entryPointIndex,
    int32_t targetIndex,
    SlangcBlob** outCode,
    SlangcBlob** outDiagnostics);

/** Release a component type.
 * @param componentType The component type to release
 */
void slangc_releaseComponentType(SlangcComponentType* componentType);

//
// Compilation (Legacy API)
//

/** Create a simple compile request (legacy interface).
 * @param session The compilation session
 * @param outCompileRequest Pointer to receive the compile request
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_createCompileRequest(
    SlangcSession* session,
    SlangcCompileRequest** outCompileRequest);

/** Add source code to a compile request.
 * @param request The compile request
 * @param language The source language
 * @param path The file path (for error reporting)
 * @param source The source code
 * @return The translation unit index, or -1 on error
 */
int32_t slangc_addTranslationUnit(
    SlangcCompileRequest* request,
    SlangcSourceLanguage language,
    const char* path,
    const char* source);

/** Add an entry point to a compile request.
 * @param request The compile request
 * @param translationUnitIndex The translation unit containing the entry point
 * @param name The entry point function name
 * @param stage The shader stage
 * @return The entry point index, or -1 on error
 */
int32_t slangc_addEntryPoint(
    SlangcCompileRequest* request,
    int32_t translationUnitIndex,
    const char* name,
    SlangcStage stage);

/** Set the target for compilation.
 * @param request The compile request
 * @param target The compilation target
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_setTarget(SlangcCompileRequest* request, SlangcCompileTarget target);

/** Perform compilation.
 * @param request The compile request
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_compile(SlangcCompileRequest* request);

/** Get compiled code from a compile request.
 * @param request The compile request
 * @param entryPointIndex The entry point index
 * @param outCode Pointer to receive the compiled code blob
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_getCompiledCode(
    SlangcCompileRequest* request,
    int32_t entryPointIndex,
    SlangcBlob** outCode);

/** Get diagnostic output from a compile request.
 * @param request The compile request
 * @param outDiagnostics Pointer to receive the diagnostic blob
 * @return SLANGC_OK on success, error code on failure
 */
SlangcResult slangc_getDiagnosticOutput(
    SlangcCompileRequest* request,
    SlangcBlob** outDiagnostics);

/** Release a compile request.
 * @param request The compile request to release
 */
void slangc_releaseCompileRequest(SlangcCompileRequest* request);

//
// Blob Management
//

/** Get the data pointer from a blob.
 * @param blob The blob
 * @return Pointer to the blob data
 */
const void* slangc_getBlobData(SlangcBlob* blob);

/** Get the size of a blob.
 * @param blob The blob
 * @return Size in bytes
 */
size_t slangc_getBlobSize(SlangcBlob* blob);

/** Release a blob.
 * @param blob The blob to release
 */
void slangc_releaseBlob(SlangcBlob* blob);

//
// Utilities
//

/** Get version information.
 * @return Version string (do not free)
 */
const char* slangc_getVersionString(void);

/** Shutdown the Slang system (call after all objects are released).
 */
void slangc_shutdown(void);

/** Get the last internal error message.
 * @return Error message string (do not free)
 */
const char* slangc_getLastErrorMessage(void);

#ifdef __cplusplus
}
#endif

#endif // SLANGC_H
