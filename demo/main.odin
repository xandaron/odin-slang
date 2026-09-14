package test

import slang "../odin-slang"
import "core:fmt"
import "core:os"

// Prints diagnostics produced by a Slang call, if any, and releases the blob.
print_diagnostics :: proc(diagnostics: ^slang.IBlob) {
	if diagnostics == nil {
		return
	}
	defer diagnostics->Release()
	fmt.eprintln(cstring(diagnostics->GetBufferPointer()))
}

write_blob_to_file :: proc(path: string, blob: ^slang.IBlob) {
	defer blob->Release()
	data := ([^]u8)(blob->GetBufferPointer())[:blob->GetBufferSize()]
	file, _ := os.open(path, os.O_WRONLY | os.O_CREATE | os.O_TRUNC)
	defer os.close(file)
	os.write(file, data)
}

main :: proc() {
	global_session: ^slang.IGlobalSession
	if slang.failed(slang.createGlobalSession(slang.API_VERSION, &global_session)) {
		panic("Failed to create global session")
	}
	defer global_session->Release()

	search_paths := []cstring{"./shaders"}

	target := slang.target_desc_default()
	target.format = .GLSL
	targets := []slang.Target_Desc{target}

	session_desc := slang.session_desc_default()
	session_desc.targets = raw_data(targets)
	session_desc.targetCount = len(targets)
	session_desc.searchPaths = raw_data(search_paths)
	session_desc.searchPathCount = len(search_paths)

	session: ^slang.ISession
	if slang.failed(global_session->CreateSession(&session_desc, &session)) {
		panic("Failed to create session")
	}
	defer session->Release()

	diagnostics: ^slang.IBlob

	// --- Vertex + fragment shaders ---

	vertex_module := session->LoadModule("./shaders/VertexDemo.slang", &diagnostics)
	print_diagnostics(diagnostics)
	if vertex_module == nil {
		panic("Failed to load module")
	}
	defer vertex_module->Release()

	vert_entry_point: ^slang.IEntryPoint
	if slang.failed(vertex_module->FindAndCheckEntryPoint("vertexMain", .VERTEX, &vert_entry_point, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to find entry point")
	}
	defer vert_entry_point->Release()

	frag_entry_point: ^slang.IEntryPoint
	if slang.failed(vertex_module->FindAndCheckEntryPoint("fragmentMain", .FRAGMENT, &frag_entry_point, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to find entry point")
	}
	defer frag_entry_point->Release()

	components := []^slang.IComponentType{vertex_module, vert_entry_point, frag_entry_point}
	program: ^slang.IComponentType
	if slang.failed(session->CreateCompositeComponentType(raw_data(components), len(components), &program, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to create program")
	}
	defer program->Release()

	linked_program: ^slang.IComponentType
	if slang.failed(program->Link(&linked_program, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to link program")
	}
	defer linked_program->Release()

	vert_code: ^slang.IBlob
	if slang.failed(linked_program->GetEntryPointCode(0, 0, &vert_code, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to get vertex entry point code")
	}
	write_blob_to_file("vert.glsl", vert_code)

	frag_code: ^slang.IBlob
	if slang.failed(linked_program->GetEntryPointCode(1, 0, &frag_code, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to get fragment entry point code")
	}
	write_blob_to_file("frag.glsl", frag_code)

	// --- Compute shader ---

	compute_module := session->LoadModule("./shaders/ComputeDemo.slang", &diagnostics)
	print_diagnostics(diagnostics)
	if compute_module == nil {
		panic("Failed to load module")
	}
	defer compute_module->Release()

	compute_entry_point: ^slang.IEntryPoint
	if slang.failed(compute_module->FindAndCheckEntryPoint("computeMain", .COMPUTE, &compute_entry_point, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to find entry point")
	}
	defer compute_entry_point->Release()

	compute_components := []^slang.IComponentType{compute_module, compute_entry_point}
	compute_program: ^slang.IComponentType
	if slang.failed(session->CreateCompositeComponentType(raw_data(compute_components), len(compute_components), &compute_program, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to create program")
	}
	defer compute_program->Release()

	linked_compute_program: ^slang.IComponentType
	if slang.failed(compute_program->Link(&linked_compute_program, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to link program")
	}
	defer linked_compute_program->Release()

	compute_code: ^slang.IBlob
	if slang.failed(linked_compute_program->GetEntryPointCode(0, 0, &compute_code, &diagnostics)) {
		print_diagnostics(diagnostics)
		panic("Failed to get compute entry point code")
	}
	write_blob_to_file("comp.glsl", compute_code)

	slang.shutdown()
}
