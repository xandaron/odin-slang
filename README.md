# Odin Slang Bindings

[![License](https://img.shields.io/badge/license-MIT-blue)](#license)

## Overview

This project provides Odin bindings for [Slang](https://github.com/shader-slang/slang), a shader programming language that extends HLSL with modern language features.

The bindings are hand-written against Slang's native **COM-lite ABI** - the same vtable-based interfaces (`ISlangUnknown`, `IGlobalSession`, `ISession`, `IModule`, `IComponentType`, `IEntryPoint`, ...) that Slang's own C++ API exposes. There is no intermediate C shim: the Odin binding calls straight into `slang.dll`/`libslang`, and interface pointers behave like their C++ counterparts.

Written against and verified working with **Slang [v2026.17.1](https://github.com/shader-slang/slang/releases/tag/v2026.17.1)**. Since this binding hand-mirrors vtable layout and struct fields, a different Slang release can silently break the ABI (added/removed/reordered virtual methods, changed struct fields) - if you target another version, diff `include/slang.h` against the copy in this repo before assuming it still lines up.

## What is Slang?

[Slang](https://github.com/shader-slang/slang) is a shading language that makes it easier to build and maintain large shader codebases. It provides:

- **Modularity** - Import/export system for sharing code across shaders
- **Generics** - Parametric polymorphism for reusable shader components
- **Interfaces** - Abstract types for flexible shader architectures
- **Multiple Targets** - Compile to HLSL, GLSL, SPIR-V, Metal, CUDA, and more
- **Reflection** - Rich metadata about shader parameters and resources

## Package layout

- **`slang/`** - the `slang` Odin package (the binding itself).
- **`include/`** - the Slang headers this binding was written against, plus Slang's own `LICENSE-slang.txt`. Kept for reference when extending the binding.
- **`demo/`** - a small end-to-end example: compiles a vertex/fragment `.slang` file and a compute `.slang` file down to GLSL.

## Using the COM-lite interfaces

Every Slang object derives from `ISlangUnknown`, and every method lives on a raw C-style vtable (an out-pointer parameter for the result, a `slang.Result`/nil return for failure) - that ABI shape is fixed by Slang itself. This package provides idiomatic Odin wrapper procs (snake_case, e.g. `->create_session(...)`) that turn each out-pointer into a normal return value plus an `ok: bool`, so you don't preallocate result variables yourself - use those rather than the raw `->MethodName(...)` form. The vtable struct types (and their UUIDs) are private to the package specifically to signal this: they aren't the intended way to interact with the library, even though Odin doesn't fully block calling through them.

Every interface (`Global_Session`, `Session`, `Module`, `Component_Type`, `Entry_Point`, `Blob`, ...) converts implicitly to `^Unknown` via Odin's `#subtype`, so instead of calling `->Release()` on each concrete type, release everything through the single `release()` proc. It's a no-op on `nil`, so it can be deferred unconditionally - even for optional outputs like diagnostics blobs that may or may not have been produced:

```odin
import slang "slang"

global_session, ok := slang.create_global_session()
if !ok {
    // handle failure
}
defer slang.release(global_session)

session_desc := slang.session_desc_default()
// ... fill in session_desc.targets / searchPaths ...

session, ok := global_session->create_session(&session_desc)
if !ok {
    // handle failure
}
defer slang.release(session)

module, diagnostics, module_ok := session->load_module("shader.slang")
defer slang.release(diagnostics)
if diagnostics != nil {
    fmt.eprintln(cstring(diagnostics->get_buffer_pointer()))
}
if !module_ok {
    // handle failure
}
defer slang.release(module)
```

Note that `Session` itself is a private type - `create_session` returns `^slang.Session`, but you hold it through `:=` type inference rather than spelling the type out. That's true of every interface handle except `Blob` and `Component_Type`, which you do name directly (e.g. in a slice literal below).

`Module` and `Entry_Point` are themselves `Component_Type`s (via Odin's `#subtype`), so they can be passed directly to `session.create_composite_component_type`:

```odin
entry_point, _, entry_point_ok := module->find_and_check_entry_point("main", .COMPUTE)
defer slang.release(entry_point)

components := []^slang.Component_Type{module, entry_point}
program, _, program_ok := session->create_composite_component_type(components)
defer slang.release(program)

linked, _, linked_ok := program->link()
defer slang.release(linked)

code, _, code_ok := linked->get_entry_point_code(0, 0)
defer slang.release(code)
```

Every wrapper proc returns `ok: bool` as its last value; where the underlying call also produces a diagnostics blob, that comes back as the second-to-last value (`nil` when there's nothing to report). Work with `slang.Result` directly via `slang.succeeded`/`slang.failed` if you need it.

See `demo/main.odin` for a complete, runnable example.

## Building

### Prerequisites

- [Odin compiler](https://odin-lang.org/)
- Prebuilt Slang libraries for your platform (see below)

### Getting Slang's libraries

This repo does not vendor Slang's compiled binaries - `*.dll`/`*.lib`/`*.so`/`*.dylib` are gitignored, so you need to supply your own copy after every fresh checkout. Download a release from the [Slang releases page](https://github.com/shader-slang/slang/releases) (or build Slang yourself from the `slang/` submodule/checkout) for your platform, then:

- **Windows**: copy `slang.lib` next to the binding (`slang/slang.lib`) so the Odin linker can find it, and copy `slang.dll` next to whatever executable links against the binding (e.g. `demo/`). Recent Slang releases split the compiler itself out into `slang-compiler.dll`, which `slang.dll` loads at runtime - copy that alongside it too. Add whichever of `slang-rt.dll`, `slang-glslang.dll`, `slang-glsl-module.dll`, `slang-llvm.dll` your target formats need (GLSL output needs `slang-glslang.dll` and `slang-glsl-module.dll`).
- **Linux/macOS**: copy `libslang.so`/`libslang.dylib` (and friends) somewhere your dynamic linker can find them (e.g. alongside the executable, or a directory on `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH`).

`slang/` is where `foreign import` resolves the `.lib` from; the runtime shared libraries need to be next to (or discoverable by) the final built executable. Since none of this is tracked in git, re-copy these files after cloning the repo fresh.

### Running the demo

```bash
cd demo
odin build . -out:test.exe
./test.exe
```

This compiles `shaders/VertexDemo.slang` and `shaders/ComputeDemo.slang` to GLSL, writing `vert.glsl`, `frag.glsl`, and `comp.glsl`.

## Extending the binding

`slang/` mirrors the subset of `include/slang.h` needed for a full module-load -> link -> emit-code pipeline (`Global_Session`, `Session`, `Component_Type`, `Module`, `Entry_Point`, `Blob` - Slang's own C++ API spells these with a leading `I`, e.g. `IGlobalSession`; this binding drops it). Reflection types (`Type_Reflection`, `Type_Layout_Reflection`, `Program_Layout`, `Function_Reflection`, `Decl_Reflection`) and a few peripheral interfaces (`Slang_File_System`, `Slang_Shared_Library`, ...) are declared as opaque pointer types - the ABI shapes are reserved but their vtables aren't bound yet.

When binding a new interface method, remember:

- Vtable order must match `include/slang.h` exactly - method names don't matter for the ABI, only position.
- Enum backing types must match the `...Integral` typedef in `slang.h` (commonly `i32` or `u32`, *not* Odin's default `int`).
- Plain-data structs (`SessionDesc`, `TargetDesc`, ...) must keep field order and types identical to their C++ definitions; Odin's default struct layout follows the same natural-alignment rules as C++, so no manual padding is needed as long as types/order match.

## License

This project's own code is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Slang itself is licensed under **Apache License 2.0 with LLVM exceptions** - see [`include/LICENSE-slang.txt`](include/LICENSE-slang.txt).

## Related Projects

- [Slang Official Repository](https://github.com/shader-slang/slang) - The Slang shading language
- [Odin Language](https://odin-lang.org/) - The Odin programming language
