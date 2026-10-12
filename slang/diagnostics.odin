package slang

import "core:fmt"

// Prints a diagnostics blob returned by a Slang call to stderr, if there is
// one, and releases it. Safe to call with nil, so you can pass the
// diagnostics output of any wrapper proc straight in.
print_diagnostics :: proc(diagnostics: ^Blob) {
	defer release(diagnostics)
	if diagnostics == nil {
		return
	}
	fmt.eprintln(cstring(get_buffer_pointer(diagnostics)))
}
