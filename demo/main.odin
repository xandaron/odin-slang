package test

import slang "../slang"
import "core:fmt"
import "core:os"

// Prints diagnostics produced by a Slang call, if any, and releases the blob.
print_diagnostics :: proc(diagnostics: ^slang.Blob) {
	defer slang.release(diagnostics)
	if diagnostics == nil {
		return
	}
	fmt.eprintln(cstring(slang.get_buffer_pointer(diagnostics)))
}

write_blob_to_file :: proc(path: string, blob: ^slang.Blob) {
	defer slang.release(blob)
	data := ([^]u8)(slang.get_buffer_pointer(blob))[:slang.get_buffer_size(blob)]
	file, _ := os.open(path, os.O_WRONLY | os.O_CREATE | os.O_TRUNC)
	defer os.close(file)
	os.write(file, data)
}

main :: proc() {
	global_session, global_session_ok := slang.create_global_session()
	if !global_session_ok {
		panic("Failed to create global session")
	}
	defer slang.release(global_session)

	search_paths := []cstring{"./shaders"}

	target := slang.target_desc_default()
	target.format = .GLSL
	targets := []slang.Target_Desc{target}

	session_desc := slang.session_desc_default()
	session_desc.targets = raw_data(targets)
	session_desc.targetCount = len(targets)
	session_desc.searchPaths = raw_data(search_paths)
	session_desc.searchPathCount = len(search_paths)

	session, session_ok := slang.create_session(global_session, &session_desc)
	if !session_ok {
		panic("Failed to create session")
	}
	defer slang.release(session)

	// --- Vertex + fragment shaders ---

	vertex_module, vertex_diagnostics, vertex_module_ok := slang.load_module(session, "./shaders/VertexDemo.slang")
	print_diagnostics(vertex_diagnostics)
	if !vertex_module_ok {
		panic("Failed to load module")
	}
	defer slang.release(vertex_module)

	vert_entry_point, vert_entry_diagnostics, vert_entry_ok := slang.find_and_check_entry_point(vertex_module, "vertexMain", .VERTEX)
	print_diagnostics(vert_entry_diagnostics)
	if !vert_entry_ok {
		panic("Failed to find entry point")
	}
	defer slang.release(vert_entry_point)

	frag_entry_point, frag_entry_diagnostics, frag_entry_ok := slang.find_and_check_entry_point(vertex_module, "fragmentMain", .FRAGMENT)
	print_diagnostics(frag_entry_diagnostics)
	if !frag_entry_ok {
		panic("Failed to find entry point")
	}
	defer slang.release(frag_entry_point)

	components := []^slang.Component_Type{vertex_module, vert_entry_point, frag_entry_point}
	program, program_diagnostics, program_ok := slang.create_composite_component_type(session, components)
	print_diagnostics(program_diagnostics)
	if !program_ok {
		panic("Failed to create program")
	}
	defer slang.release(program)

	linked_program, linked_program_diagnostics, linked_program_ok := slang.link(program)
	print_diagnostics(linked_program_diagnostics)
	if !linked_program_ok {
		panic("Failed to link program")
	}
	defer slang.release(linked_program)

	vert_code, vert_code_diagnostics, vert_code_ok := slang.get_entry_point_code(linked_program, 0, 0)
	print_diagnostics(vert_code_diagnostics)
	if !vert_code_ok {
		panic("Failed to get vertex entry point code")
	}
	write_blob_to_file("vert.glsl", vert_code)

	frag_code, frag_code_diagnostics, frag_code_ok := slang.get_entry_point_code(linked_program, 1, 0)
	print_diagnostics(frag_code_diagnostics)
	if !frag_code_ok {
		panic("Failed to get fragment entry point code")
	}
	write_blob_to_file("frag.glsl", frag_code)

	// --- Compute shader ---

	compute_module, compute_diagnostics, compute_module_ok := slang.load_module(session, "./shaders/ComputeDemo.slang")
	print_diagnostics(compute_diagnostics)
	if !compute_module_ok {
		panic("Failed to load module")
	}
	defer slang.release(compute_module)

	compute_entry_point, compute_entry_diagnostics, compute_entry_ok := slang.find_and_check_entry_point(compute_module, "computeMain", .COMPUTE)
	print_diagnostics(compute_entry_diagnostics)
	if !compute_entry_ok {
		panic("Failed to find entry point")
	}
	defer slang.release(compute_entry_point)

	compute_components := []^slang.Component_Type{compute_module, compute_entry_point}
	compute_program, compute_program_diagnostics, compute_program_ok := slang.create_composite_component_type(session, compute_components)
	print_diagnostics(compute_program_diagnostics)
	if !compute_program_ok {
		panic("Failed to create program")
	}
	defer slang.release(compute_program)

	linked_compute_program, linked_compute_diagnostics, linked_compute_ok := slang.link(compute_program)
	print_diagnostics(linked_compute_diagnostics)
	if !linked_compute_ok {
		panic("Failed to link program")
	}
	defer slang.release(linked_compute_program)

	compute_code, compute_code_diagnostics, compute_code_ok := slang.get_entry_point_code(linked_compute_program, 0, 0)
	print_diagnostics(compute_code_diagnostics)
	if !compute_code_ok {
		panic("Failed to get compute entry point code")
	}
	write_blob_to_file("comp.glsl", compute_code)

	slang.shutdown()
}
