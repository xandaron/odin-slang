package slang

import "core:c"

_ :: c

when ODIN_OS == .Windows {
    foreign import lib {
        "gfx.lib",
        "slang-rt.lib",
        "slang.lib",
    }
}
else {
    foreign import lib {
        "gfx.a",
        "slang-rt.a",
        "slang.a",
    }
}

OK :: 0

// Result codes
Result :: i32

FAIL :: -1
E_NOT_IMPLEMENTED :: -2
E_INVALID_ARG :: -3
E_OUT_OF_MEMORY :: -4
E_BUFFER_TOO_SMALL :: -5

// API version
API_VERSION :: 1

// Source languages
Source_Language :: enum c.int {
	UNKNOWN = 0,
	SLANG,
	HLSL,
	GLSL,
	C,
	CPP,
}

// Compilation targets
Compile_Target :: enum c.int {
	UNKNOWN = 0,
	NONE,
	HLSL,
	GLSL,
	SPIRV,
	SPIRV_ASM,
	DXBC,
	DXBC_ASM,
	DXIL,
	DXIL_ASM,
	C_SOURCE,
	CPP_SOURCE,
	CUDA_SOURCE,
	PTX,
	CUBIN,
	METAL,
	METAL_LIB,
	HOST_CALLABLE,
	SHADER_SHARED_LIBRARY,
	SHADER_HOST_CALLABLE,
	WGSL,
}

// Stage types
Stage :: enum c.int {
	NONE = 0,
	VERTEX,
	HULL,
	DOMAIN,
	GEOMETRY,
	FRAGMENT,
	COMPUTE,
	RAY_GENERATION,
	INTERSECTION,
	ANY_HIT,
	CLOSEST_HIT,
	MISS,
	CALLABLE,
	MESH,
	AMPLIFICATION,
}

// Profile IDs (opaque)
Profile_Id :: u32

// Session description
Session_Desc :: struct {
	targets:                ^Compile_Target,
	targetCount:            i32,
	searchPaths:            [^]cstring,
	searchPathCount:        i32,
	preprocessorMacros:     [^]cstring,
	preprocessorMacroCount: i32,
}

// Global session description
Global_Session_Desc :: struct {
	apiVersion:      u32,
	searchPaths:     [^]cstring,
	searchPathCount: i32,
	enableGLSL:      bool,
}

@(default_calling_convention="c", link_prefix="slangc_")
foreign lib {
	/** Create a global session with default settings.
	* @param outGlobalSession Pointer to receive the created global session
	* @return SLANGC_OK on success, error code on failure
	*/
	createGlobalSession :: proc(outGlobalSession: ^^Global_Session) -> Result ---

	/** Create a global session with custom settings.
	* @param desc Description of the global session configuration
	* @param outGlobalSession Pointer to receive the created global session
	* @return SLANGC_OK on success, error code on failure
	*/
	createGlobalSessionWithDesc :: proc(desc: ^Global_Session_Desc, outGlobalSession: ^^Global_Session) -> Result ---

	/** Release a global session and free its resources.
	* @param globalSession The global session to release
	*/
	releaseGlobalSession :: proc(globalSession: ^Global_Session) ---

	/** Find a profile by name.
	* @param globalSession The global session
	* @param name The profile name (e.g., "vs_5_0", "ps_5_0", "cs_5_0")
	* @return The profile ID, or 0 if not found
	*/
	findProfile :: proc(globalSession: ^Global_Session, name: cstring) -> Profile_Id ---

	/** Create a compilation session.
	* @param globalSession The global session
	* @param desc Session description (can be NULL for defaults)
	* @param outSession Pointer to receive the created session
	* @return SLANGC_OK on success, error code on failure
	*/
	createSession :: proc(globalSession: ^Global_Session, desc: ^Session_Desc, outSession: ^^Session) -> Result ---

	/** Release a session and free its resources.
	* @param session The session to release
	*/
	releaseSession :: proc(session: ^Session) ---

	/** Load a module by name (for import statements).
	* @param session The compilation session
	* @param moduleName The name of the module to load
	* @param outModule Pointer to receive the loaded module
	* @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
	* @return SLANGC_OK on success, error code on failure
	*/
	loadModule :: proc(session: ^Session, moduleName: cstring, outModule: ^^Module, outDiagnostics: ^^Blob) -> Result ---

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
	loadModuleFromSource :: proc(session: ^Session, moduleName: cstring, path: cstring, sourceText: cstring, sourceSize: c.size_t, outModule: ^^Module, outDiagnostics: ^^Blob) -> Result ---

	/** Release a module.
	* @param module The module to release
	*/
	releaseModule :: proc(module: ^Module) ---

	/** Create a composite component type from multiple components.
	* @param session The compilation session
	* @param componentTypes Array of component types to combine
	* @param componentTypeCount Number of component types
	* @param outComposite Pointer to receive the composite component type
	* @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
	* @return SLANGC_OK on success, error code on failure
	*/
	createCompositeComponentType :: proc(session: ^Session, componentTypes: ^^Component_Type, componentTypeCount: i32, outComposite: ^^Component_Type, outDiagnostics: ^^Blob) -> Result ---

	/** Find and check an entry point in a module.
	* @param module The loaded module
	* @param entryPointName The name of the entry point function
	* @param stage The shader stage for the entry point
	* @param outEntryPoint Pointer to receive the entry point
	* @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
	* @return SLANGC_OK on success, error code on failure
	*/
	findEntryPoint :: proc(module: ^Module, entryPointName: cstring, stage: Stage, outEntryPoint: ^^Entry_Point, outDiagnostics: ^^Blob) -> Result ---

	/** Release an entry point.
	* @param entryPoint The entry point to release
	*/
	releaseEntryPoint :: proc(entryPoint: ^Entry_Point) ---

	/** Create a component type from a module and entry point.
	* @param session The compilation session
	* @param module The module containing the entry point
	* @param entryPoint The entry point
	* @param outComponentType Pointer to receive the component type
	* @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
	* @return SLANGC_OK on success, error code on failure
	*/
	createModuleComponentType :: proc(session: ^Session, module: ^Module, entryPoint: ^Entry_Point, outComponentType: ^^Component_Type, outDiagnostics: ^^Blob) -> Result ---

	/** Get entry point code for a specific entry point and target.
	* @param componentType The component type to compile
	* @param entryPointIndex The entry point index (0-based)
	* @param targetIndex The target index (0-based, from session targets)
	* @param outCode Pointer to receive the compiled code
	* @param outDiagnostics Pointer to receive diagnostic messages (can be NULL)
	* @return SLANGC_OK on success, error code on failure
	*/
	getEntryPointCode :: proc(componentType: ^Component_Type, entryPointIndex: i32, targetIndex: i32, outCode: ^^Blob, outDiagnostics: ^^Blob) -> Result ---

	/** Release a component type.
	* @param componentType The component type to release
	*/
	releaseComponentType :: proc(componentType: ^Component_Type) ---

	/** Create a simple compile request (legacy interface).
	* @param session The compilation session
	* @param outCompileRequest Pointer to receive the compile request
	* @return SLANGC_OK on success, error code on failure
	*/
	createCompileRequest :: proc(session: ^Session, outCompileRequest: ^^Compile_Request) -> Result ---

	/** Add source code to a compile request.
	* @param request The compile request
	* @param language The source language
	* @param path The file path (for error reporting)
	* @param source The source code
	* @return The translation unit index, or -1 on error
	*/
	addTranslationUnit :: proc(request: ^Compile_Request, language: Source_Language, path: cstring, source: cstring) -> i32 ---

	/** Add an entry point to a compile request.
	* @param request The compile request
	* @param translationUnitIndex The translation unit containing the entry point
	* @param name The entry point function name
	* @param stage The shader stage
	* @return The entry point index, or -1 on error
	*/
	addEntryPoint :: proc(request: ^Compile_Request, translationUnitIndex: i32, name: cstring, stage: Stage) -> i32 ---

	/** Set the target for compilation.
	* @param request The compile request
	* @param target The compilation target
	* @return SLANGC_OK on success, error code on failure
	*/
	setTarget :: proc(request: ^Compile_Request, target: Compile_Target) -> Result ---

	/** Perform compilation.
	* @param request The compile request
	* @return SLANGC_OK on success, error code on failure
	*/
	compile :: proc(request: ^Compile_Request) -> Result ---

	/** Get compiled code from a compile request.
	* @param request The compile request
	* @param entryPointIndex The entry point index
	* @param outCode Pointer to receive the compiled code blob
	* @return SLANGC_OK on success, error code on failure
	*/
	getCompiledCode :: proc(request: ^Compile_Request, entryPointIndex: i32, outCode: ^^Blob) -> Result ---

	/** Get diagnostic output from a compile request.
	* @param request The compile request
	* @param outDiagnostics Pointer to receive the diagnostic blob
	* @return SLANGC_OK on success, error code on failure
	*/
	getDiagnosticOutput :: proc(request: ^Compile_Request, outDiagnostics: ^^Blob) -> Result ---

	/** Release a compile request.
	* @param request The compile request to release
	*/
	releaseCompileRequest :: proc(request: ^Compile_Request) ---

	/** Get the data pointer from a blob.
	* @param blob The blob
	* @return Pointer to the blob data
	*/
	getBlobData :: proc(blob: ^Blob) -> rawptr ---

	/** Get the size of a blob.
	* @param blob The blob
	* @return Size in bytes
	*/
	getBlobSize :: proc(blob: ^Blob) -> c.size_t ---

	/** Release a blob.
	* @param blob The blob to release
	*/
	releaseBlob :: proc(blob: ^Blob) ---

	/** Get version information.
	* @return Version string (do not free)
	*/
	getVersionString :: proc() -> cstring ---

	/** Shutdown the Slang system (call after all objects are released).
	*/
	shutdown :: proc() ---

	/** Get the last internal error message.
	* @return Error message string (do not free)
	*/
	getLastErrorMessage :: proc() -> cstring ---
}
