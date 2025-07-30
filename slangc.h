#ifndef SLANGC_H
#define SLANGC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

    // Component type - can be either a module, entry point, or composite
    typedef enum SlangcComponentTypeKind
    {
        SLANGC_COMPONENT_TYPE_MODULE,
        SLANGC_COMPONENT_TYPE_ENTRY_POINT,
        SLANGC_COMPONENT_TYPE_COMPOSITE
    } SlangcComponentTypeKind;

    // Component type struct - exposed so users can access it directly
    typedef struct SlangcComponentType
    {
        SlangcComponentTypeKind kind;
        union
        {
            struct SlangcModule *module;
            struct SlangcEntryPoint *entryPoint;
            void *composite; // Opaque pointer to internal composite type
        };
    } SlangcComponentType;

    // Forward declarations for opaque handles
    typedef struct SlangcGlobalSession SlangcGlobalSession;
    typedef struct SlangcSession SlangcSession;
    typedef struct SlangcModule SlangcModule;
    typedef struct SlangcEntryPoint SlangcEntryPoint;
    typedef struct SlangcBlob SlangcBlob;

    // Result codes (still useful for some functions)
    typedef int32_t SlangcResult;
#define SLANGC_OK 0
#define SLANGC_FAIL -1
#define SLANGC_E_NOT_IMPLEMENTED -2
#define SLANGC_E_INVALID_ARG -3
#define SLANGC_E_OUT_OF_MEMORY -4
#define SLANGC_E_BUFFER_TOO_SMALL -5

    // Error handling - for functions that return pointers directly
    /** Check if the last operation succeeded.
     * @return true if the last operation succeeded, false otherwise
     */
    bool slangc_hasError(void);

    /** Get the last error message.
     * @return Error message string (do not free), or NULL if no error
     */
    const char *slangc_getLastError(void);

    /** Clear the last error state.
     */
    void slangc_clearError(void);

// API version
#define SLANGC_API_VERSION 1

    // Source languages
    typedef enum SlangcSourceLanguage
    {
        SLANGC_SOURCE_LANGUAGE_UNKNOWN = 0,
        SLANGC_SOURCE_LANGUAGE_SLANG,
        SLANGC_SOURCE_LANGUAGE_HLSL,
        SLANGC_SOURCE_LANGUAGE_GLSL,
        SLANGC_SOURCE_LANGUAGE_C,
        SLANGC_SOURCE_LANGUAGE_CPP
    } SlangcSourceLanguage;

    // Compilation targets
    typedef enum SlangcCompileTarget
    {
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
    typedef enum SlangcStage
    {
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

    // Matrix layout modes
    typedef enum SlangcMatrixLayoutMode
    {
        SLANGC_MATRIX_LAYOUT_ROW_MAJOR = 0,
        SLANGC_MATRIX_LAYOUT_COLUMN_MAJOR = 1
    } SlangcMatrixLayoutMode;

    // Session description
    typedef struct SlangcSessionDesc
    {
        SlangcCompileTarget *targets;
        int32_t targetCount;
        const char **searchPaths;
        int32_t searchPathCount;
        const char **preprocessorMacros;
        int32_t preprocessorMacroCount;
        SlangcMatrixLayoutMode matrixLayoutMode;
    } SlangcSessionDesc;

    // Global session description
    typedef struct SlangcGlobalSessionDesc
    {
        uint32_t apiVersion;
        const char **searchPaths;
        int32_t searchPathCount;
        bool enableGLSL;
    } SlangcGlobalSessionDesc;

    //
    // Global Session Management
    //

    /** Create a global session with default settings.
     * @return Pointer to the created global session, or NULL on failure
     */
    SlangcGlobalSession *slangc_createGlobalSession(void);

    /** Create a global session with custom settings.
     * @param desc Description of the global session configuration
     * @return Pointer to the created global session, or NULL on failure
     */
    SlangcGlobalSession *slangc_createGlobalSessionWithDesc(
        const SlangcGlobalSessionDesc *desc);

    /** Release a global session and free its resources.
     * @param globalSession The global session to release
     */
    void slangc_releaseGlobalSession(SlangcGlobalSession *globalSession);

    /** Find a profile by name.
     * @param globalSession The global session
     * @param name The profile name (e.g., "vs_5_0", "ps_5_0", "cs_5_0")
     * @return The profile ID, or 0 if not found
     */
    SlangcProfileID slangc_findProfile(SlangcGlobalSession *globalSession, const char *name);

    //
    // Session Management
    //

    /** Create a compilation session.
     * @param globalSession The global session
     * @param desc Session description (can be NULL for defaults)
     * @return Pointer to the created session, or NULL on failure
     */
    SlangcSession *slangc_createSession(
        SlangcGlobalSession *globalSession,
        const SlangcSessionDesc *desc);

    /** Create a compilation session with a specific profile.
     * @param globalSession The global session
     * @param profile The profile ID to use (from slangc_findProfile)
     * @param desc Session description (can be NULL for defaults)
     * @return Pointer to the created session, or NULL on failure
     */
    SlangcSession *slangc_createSessionWithProfile(
        SlangcGlobalSession *globalSession,
        SlangcProfileID profile,
        const SlangcSessionDesc *desc);

    /** Release a session and free its resources.
     * @param session The session to release
     */
    void slangc_releaseSession(SlangcSession *session);

    //
    // Module Loading
    //

    /** Load a module by name (for import statements).
     * @param session The compilation session
     * @param moduleName The name of the module to load
     * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
     * @return Pointer to the loaded module, or NULL on failure
     */
    SlangcModule *slangc_loadModule(
        SlangcSession *session,
        const char *moduleName,
        SlangcBlob **outDiagnostics);

    /** Load a module from source code.
     * @param session The compilation session
     * @param moduleName The name to give the module
     * @param path The file path (for error reporting)
     * @param sourceText The source code text
     * @param sourceSize The size of the source code (or 0 if null-terminated)
     * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
     * @return Pointer to the loaded module, or NULL on failure
     */
    SlangcModule *slangc_loadModuleFromSource(
        SlangcSession *session,
        const char *moduleName,
        const char *path,
        const char *sourceText,
        size_t sourceSize,
        SlangcBlob **outDiagnostics);

    /** Release a module.
     * @param module The module to release
     */
    void slangc_releaseModule(SlangcModule *module);

    //
    // Component Type Management
    //

    /** Get the kind of a component type.
     * @param componentType The component type to query
     * @return The kind of component type, or SLANGC_COMPONENT_TYPE_MODULE if invalid
     */
    SlangcComponentTypeKind slangc_getComponentTypeKind(SlangcComponentType *componentType);

    /** Get the underlying module from a module component type.
     * @param componentType The component type (must be SLANGC_COMPONENT_TYPE_MODULE)
     * @return Pointer to the module, or NULL if not a module component type
     */
    SlangcModule *slangc_getComponentTypeModule(SlangcComponentType *componentType);

    /** Get the underlying entry point from an entry point component type.
     * @param componentType The component type (must be SLANGC_COMPONENT_TYPE_ENTRY_POINT)
     * @return Pointer to the entry point, or NULL if not an entry point component type
     */
    SlangcEntryPoint *slangc_getComponentTypeEntryPoint(SlangcComponentType *componentType);

    /** Create a composite component type from multiple components.
     * @param session The compilation session
     * @param componentTypes Array of component type structs to combine
     * @param componentTypeCount Number of component types
     * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
     * @return Pointer to the composite component type, or NULL on failure
     */
    SlangcComponentType *slangc_createCompositeComponentType(
        SlangcSession *session,
        const SlangcComponentType *componentTypes,
        int32_t componentTypeCount,
        SlangcBlob **outDiagnostics);

    /** Find and check an entry point in a module.
     * @param module The loaded module
     * @param entryPointName The name of the entry point function
     * @param stage The shader stage for the entry point
     * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
     * @return Pointer to the entry point, or NULL on failure
     */
    SlangcEntryPoint *slangc_findEntryPoint(
        SlangcModule *module,
        const char *entryPointName,
        SlangcStage stage,
        SlangcBlob **outDiagnostics);

    /** Release an entry point.
     * @param entryPoint The entry point to release
     */
    void slangc_releaseEntryPoint(SlangcEntryPoint *entryPoint);

    /** Create a component type from a module.
     * @param module The module to wrap as a component type
     * @return Pointer to the component type, or NULL on failure
     */
    SlangcComponentType *slangc_createModuleComponentType(SlangcModule *module);

    /** Create a component type from an entry point.
     * @param entryPoint The entry point to wrap as a component type
     * @return Pointer to the component type, or NULL on failure
     */
    SlangcComponentType *slangc_createEntryPointComponentType(SlangcEntryPoint *entryPoint);

    /** Link a component type to produce a linked program.
     * @param componentType The component type to link
     * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
     * @return Pointer to the linked component type, or NULL on failure
     */
    SlangcComponentType *slangc_linkComponentType(
        SlangcComponentType *componentType,
        SlangcBlob **outDiagnostics);

    /** Get entry point code for a specific entry point and target.
     * @param componentType The component type to compile (must be fully linked)
     * @param entryPointIndex The entry point index (0-based). For simple cases with one entry point, use 0.
     *                        For composite types, this corresponds to the order entry points were added.
     * @param targetIndex The target index (0-based, from session targets)
     * @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
     * @return Pointer to the compiled code blob, or NULL on failure
     */
    SlangcBlob *slangc_getEntryPointCode(
        SlangcComponentType *componentType,
        int32_t entryPointIndex,
        int32_t targetIndex,
        SlangcBlob **outDiagnostics);

    /** Release a component type.
     * @param componentType The component type to release
     */
    void slangc_releaseComponentType(SlangcComponentType *componentType);

    //
    // Blob Management
    //

    //
    // Blob Management
    //

    /** Get the data pointer from a blob.
     * @param blob The blob
     * @return Pointer to the blob data
     */
    const void *slangc_getBlobData(SlangcBlob *blob);

    /** Get the size of a blob.
     * @param blob The blob
     * @return Size in bytes
     */
    size_t slangc_getBlobSize(SlangcBlob *blob);

    /** Release a blob.
     * @param blob The blob to release
     */
    void slangc_releaseBlob(SlangcBlob *blob);

    //
    // Utilities
    //

    /** Get version information.
     * @return Version string (do not free)
     */
    const char *slangc_getVersionString(void);

    /** Check if Slang has been shut down.
     * @return true if shutdown has been called, false otherwise
     */
    bool slangc_isShutdown(void);

    /** Shutdown the Slang system (optional cleanup - call only at application exit).
     *
     * WARNING: After calling this function, you cannot use any Slang functionality
     * again in the current process. This function performs global cleanup of Slang's
     * internal state including memory pools and module caches.
     *
     * This function is OPTIONAL and only needed if you want to clean up global
     * allocations before your application exits (e.g., to avoid false positives
     * in memory leak detectors).
     *
     * Safe to call multiple times - subsequent calls will be ignored.
     *
     * Typical usage:
     * - Create and use Slang objects normally
     * - Release all Slang objects (sessions, modules, etc.)
     * - Call this function once at application shutdown (optional)
     */
    void slangc_shutdown(void);

    /** Get the last internal error message.
     * @return Error message string (do not free)
     */
    const char *slangc_getLastErrorMessage(void);

#ifdef __cplusplus
}
#endif

#endif // SLANGC_H
