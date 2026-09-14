package slang

// A session provides a scope for loaded/compiled code: active search paths,
// global preprocessor definitions, and a list of compilation targets.
//
// Code loaded and compiled within a session is owned by the session and
// stays resident until the session is released. Use multiple sessions to
// control the memory usage of compiled/loaded code independently.
ISession :: struct #raw_union {
	#subtype unknown: IUnknown,
	using vtable:     ^ISession_VTable,
}

ISession_VTable :: struct {
	using unknown_vtable: IUnknown_VTable,

	// Get the global session that was used to create this session.
	GetGlobalSession: proc "system" (this: ^ISession) -> ^IGlobalSession,

	// Load a module as it would be by code using `import`.
	LoadModule: proc "system" (this: ^ISession, module_name: cstring, out_diagnostics: ^^IBlob) -> ^IModule,

	// Load a module from Slang source code. If `source` is nil and `path`
	// names a readable file, the module is loaded from that file's contents.
	LoadModuleFromSource: proc "system" (this: ^ISession, module_name: cstring, path: cstring, source: ^IBlob, out_diagnostics: ^^IBlob) -> ^IModule,

	// Combine multiple component types (modules, entry points, ...) into a
	// composite component type. See slang.h for the full semantics around
	// parameter/requirement ordering and deduplication.
	CreateCompositeComponentType: proc "system" (this: ^ISession, component_types: [^]^IComponentType, component_type_count: Int, out_composite_component_type: ^^IComponentType, out_diagnostics: ^^IBlob) -> Result,

	// Specialize a type based on type arguments.
	SpecializeType: proc "system" (this: ^ISession, type: ^Type_Reflection, specialization_args: [^]Specialization_Arg, specialization_arg_count: Int, out_diagnostics: ^^IBlob) -> ^Type_Reflection,

	// Get the layout of `type` on the chosen `target_index`.
	GetTypeLayout: proc "system" (this: ^ISession, type: ^Type_Reflection, target_index: Int, rules: Layout_Rules, out_diagnostics: ^^IBlob) -> ^Type_Layout_Reflection,

	// Get a container type from `element_type`, e.g. T -> StructuredBuffer<T>.
	GetContainerType: proc "system" (this: ^ISession, element_type: ^Type_Reflection, container_type: Container_Type, out_diagnostics: ^^IBlob) -> ^Type_Reflection,

	// Returns a TypeReflection representing the `__Dynamic` type, usable as a
	// specialization argument to request dynamic dispatch.
	GetDynamicType: proc "system" (this: ^ISession) -> ^Type_Reflection,

	GetTypeRTTIMangledName: proc "system" (this: ^ISession, type: ^Type_Reflection, out_name_blob: ^^IBlob) -> Result,

	GetTypeConformanceWitnessMangledName: proc "system" (this: ^ISession, type: ^Type_Reflection, interface_type: ^Type_Reflection, out_name_blob: ^^IBlob) -> Result,

	GetTypeConformanceWitnessSequentialID: proc "system" (this: ^ISession, type: ^Type_Reflection, interface_type: ^Type_Reflection, out_id: ^u32) -> Result,

	// DEPRECATED.
	CreateCompileRequest: proc "system" (this: ^ISession, out_compile_request: ^^Compile_Request) -> Result,

	// Creates an IComponentType representing a type's conformance to an
	// interface. Pass -1 for `conformance_id_override` to let Slang assign
	// dispatch IDs automatically.
	CreateTypeConformanceComponentType: proc "system" (this: ^ISession, type: ^Type_Reflection, interface_type: ^Type_Reflection, out_conformance: ^^ITypeConformance, conformance_id_override: Int, out_diagnostics: ^^IBlob) -> Result,

	// Load a module from a Slang module blob (a `.slang-module` container).
	LoadModuleFromIRBlob: proc "system" (this: ^ISession, module_name: cstring, path: cstring, source: ^IBlob, out_diagnostics: ^^IBlob) -> ^IModule,

	GetLoadedModuleCount: proc "system" (this: ^ISession) -> Int,
	GetLoadedModule:      proc "system" (this: ^ISession, index: Int) -> ^IModule,

	IsBinaryModuleUpToDate: proc "system" (this: ^ISession, module_path: cstring, binary_module_blob: ^IBlob) -> Bool,

	// Load a module from a string.
	LoadModuleFromSourceString: proc "system" (this: ^ISession, module_name: cstring, path: cstring, source_string: cstring, out_diagnostics: ^^IBlob) -> ^IModule,

	// Get the 16-byte RTTI header to fill into a dynamic object. See slang.h
	// for the full explanation and layout of the dynamic-object convention.
	GetDynamicObjectRTTIBytes: proc "system" (this: ^ISession, type: ^Type_Reflection, interface_type: ^Type_Reflection, out_rtti_data_buffer: [^]u32, buffer_size_in_bytes: u32) -> Result,

	// Read module info (name and version) from a module blob. The returned
	// pointers are valid for as long as the session.
	LoadModuleInfoFromIRBlob: proc "system" (this: ^ISession, source: ^IBlob, out_module_version: ^Int, out_module_compiler_version: ^cstring, out_module_name: ^cstring) -> Result,

	// Get the source location of a declaration. The returned filePath
	// pointer is valid for as long as the session.
	GetDeclSourceLocation: proc "system" (this: ^ISession, decl: ^Decl_Reflection, out_location: ^Source_Location) -> Result,
}

ISession_UUID := UUID{0x67618701, 0xd116, 0x468f, {0xab, 0x3b, 0x47, 0x4b, 0xed, 0xce, 0x0e, 0x3d}}
