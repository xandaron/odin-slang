package slang

// A global session for interacting with the Slang library. Base cost of
// standing up the compiler (e.g. loading the core module) is paid once per
// global session, so applications should create one and reuse it to create
// multiple ISession instances.
//
// Not thread-safe: a global session and the objects created from it should be
// externally synchronized when shared across threads. Distinct global
// sessions may be used from different threads in parallel.
IGlobalSession :: struct #raw_union {
	#subtype unknown: IUnknown,
	using vtable:     ^IGlobalSession_VTable,
}

IGlobalSession_VTable :: struct {
	using unknown_vtable: IUnknown_VTable,

	// Create a new session for loading and compiling code.
	CreateSession: proc "system" (this: ^IGlobalSession, desc: ^Session_Desc, out_session: ^^ISession) -> Result,

	// Look up the internal ID of a profile by its `name` (e.g. "vs_5_0", "glsl_450").
	// Profile IDs are not guaranteed to be stable across Slang versions.
	FindProfile: proc "system" (this: ^IGlobalSession, name: cstring) -> Profile_ID,

	SetDownstreamCompilerPath: proc "system" (this: ^IGlobalSession, pass_through: Pass_Through, path: cstring),

	// DEPRECATED: use SetLanguagePrelude.
	SetDownstreamCompilerPrelude: proc "system" (this: ^IGlobalSession, pass_through: Pass_Through, prelude_text: cstring),
	// DEPRECATED: use GetLanguagePrelude.
	GetDownstreamCompilerPrelude: proc "system" (this: ^IGlobalSession, pass_through: Pass_Through, out_prelude: ^^IBlob),

	GetBuildTagString: proc "system" (this: ^IGlobalSession) -> cstring,

	SetDefaultDownstreamCompiler: proc "system" (this: ^IGlobalSession, source_language: Source_Language, default_compiler: Pass_Through) -> Result,
	GetDefaultDownstreamCompiler: proc "system" (this: ^IGlobalSession, source_language: Source_Language) -> Pass_Through,

	SetLanguagePrelude: proc "system" (this: ^IGlobalSession, source_language: Source_Language, prelude_text: cstring),
	GetLanguagePrelude: proc "system" (this: ^IGlobalSession, source_language: Source_Language, out_prelude: ^^IBlob),

	// DEPRECATED.
	CreateCompileRequest: proc "system" (this: ^IGlobalSession, out_compile_request: ^^Compile_Request) -> Result,
	// DEPRECATED.
	AddBuiltins: proc "system" (this: ^IGlobalSession, source_path: cstring, source_string: cstring),

	SetSharedLibraryLoader: proc "system" (this: ^IGlobalSession, loader: ^ISlangSharedLibraryLoader),
	GetSharedLibraryLoader: proc "system" (this: ^IGlobalSession) -> ^ISlangSharedLibraryLoader,

	CheckCompileTargetSupport: proc "system" (this: ^IGlobalSession, target: Compile_Target) -> Result,
	CheckPassThroughSupport:   proc "system" (this: ^IGlobalSession, pass_through: Pass_Through) -> Result,

	// EXPERIMENTAL.
	CompileCoreModule: proc "system" (this: ^IGlobalSession, flags: Compile_Core_Module_Flags) -> Result,
	// EXPERIMENTAL.
	LoadCoreModule: proc "system" (this: ^IGlobalSession, core_module: rawptr, core_module_size_in_bytes: uint) -> Result,
	// EXPERIMENTAL.
	SaveCoreModule: proc "system" (this: ^IGlobalSession, archive_type: Archive_Type, out_blob: ^^IBlob) -> Result,

	FindCapability: proc "system" (this: ^IGlobalSession, name: cstring) -> Capability_ID,

	SetDownstreamCompilerForTransition: proc "system" (this: ^IGlobalSession, source: Compile_Target, target: Compile_Target, compiler: Pass_Through),
	GetDownstreamCompilerForTransition: proc "system" (this: ^IGlobalSession, source: Compile_Target, target: Compile_Target) -> Pass_Through,

	GetCompilerElapsedTime: proc "system" (this: ^IGlobalSession, out_total_time: ^f64, out_downstream_time: ^f64),

	SetSPIRVCoreGrammar: proc "system" (this: ^IGlobalSession, json_path: cstring) -> Result,

	ParseCommandLineArguments: proc "system" (this: ^IGlobalSession, argc: i32, argv: [^]cstring, out_session_desc: ^Session_Desc, out_aux_allocation: ^^IUnknown) -> Result,

	GetSessionDescDigest: proc "system" (this: ^IGlobalSession, session_desc: ^Session_Desc, out_blob: ^^IBlob) -> Result,

	// EXPERIMENTAL.
	CompileBuiltinModule: proc "system" (this: ^IGlobalSession, module: Builtin_Module_Name, flags: Compile_Core_Module_Flags) -> Result,
	// EXPERIMENTAL.
	LoadBuiltinModule: proc "system" (this: ^IGlobalSession, module: Builtin_Module_Name, module_data: rawptr, size_in_bytes: uint) -> Result,
	// EXPERIMENTAL.
	SaveBuiltinModule: proc "system" (this: ^IGlobalSession, module: Builtin_Module_Name, archive_type: Archive_Type, out_blob: ^^IBlob) -> Result,

	// Get the on-disk path of the downstream compiler Slang resolves for `pass_through`.
	GetDownstreamCompilerPath: proc "system" (this: ^IGlobalSession, pass_through: Pass_Through, out_path: ^^IBlob) -> Result,
}

IGlobalSession_UUID := UUID{0xc140b5fd, 0x0c78, 0x452e, {0xba, 0x7c, 0x1a, 0x1e, 0x70, 0xc7, 0xf7, 0x1c}}
