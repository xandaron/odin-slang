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

## Usage

Every Slang object derives from `ISlangUnknown`, and every method lives on a raw C-style vtable (an out-pointer parameter for the result, a `slang.Result` return for failure) - that ABI shape is fixed by Slang itself. This package wraps each vtable method in an idiomatic snake_case **free procedure** that takes the object as its first argument and turns out-pointers into ordinary return values plus an `ok: bool`:

```odin
// C++:  session->loadModule("shader", diagnostics.writeRef());
module, diagnostics, ok := slang.load_module(session, "shader")
```

The vtable structs (and their UUIDs) are private to the package: the wrapper procs are the API, and calling through `->MethodName(...)` isn't supported.

### Return values

- Every wrapper that can fail returns `ok: bool` as its **last** value. If you need the raw `slang.Result`, check it with `slang.succeeded`/`slang.failed`.
- If the underlying call can produce diagnostics, they come back as a `^slang.Blob` in the **second-to-last** position, or `nil` when there's nothing to report. Pass that blob to `slang.print_diagnostics`, which prints it to stderr if there is one and then releases it. It accepts `nil`, so you can call it on every diagnostics result without checking first.

### Lifetime

Every interface (`Global_Session`, `Session`, `Module`, `Component_Type`, `Entry_Point`, `Blob`, ...) converts implicitly to `^slang.Unknown` via Odin's `#subtype`, so you release all of them with one proc, `slang.release`. It's a no-op on `nil`, so you can `defer` it unconditionally, even for optional outputs like diagnostics blobs. Once every object has been released you can optionally call `slang.shutdown()` to free Slang's global allocations (e.g. to keep leak detectors quiet).

### Example

Create a global session and a session targeting GLSL:

```odin
import slang "slang"

global_session, global_session_ok := slang.create_global_session()
if !global_session_ok {
    // handle failure
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
    // handle failure
}
defer slang.release(session)
```

Load a module, find an entry point, then compose, link, and emit code:

```odin
module, module_diagnostics, module_ok := slang.load_module(session, "ComputeDemo")
slang.print_diagnostics(module_diagnostics)
if !module_ok {
    // handle failure
}
defer slang.release(module)

entry_point, entry_diagnostics, entry_ok := slang.find_and_check_entry_point(module, "computeMain", .COMPUTE)
slang.print_diagnostics(entry_diagnostics)
if !entry_ok {
    // handle failure
}
defer slang.release(entry_point)

// Module and Entry_Point are Component_Types via #subtype, so they go
// straight into the slice.
components := []^slang.Component_Type{module, entry_point}
program, program_diagnostics, program_ok := slang.create_composite_component_type(session, components)
slang.print_diagnostics(program_diagnostics)
if !program_ok {
    // handle failure
}
defer slang.release(program)

linked, link_diagnostics, link_ok := slang.link(program)
slang.print_diagnostics(link_diagnostics)
if !link_ok {
    // handle failure
}
defer slang.release(linked)

code, code_diagnostics, code_ok := slang.get_entry_point_code(linked, 0, 0)
slang.print_diagnostics(code_diagnostics)
if !code_ok {
    // handle failure
}
defer slang.release(code)

glsl := string(([^]u8)(slang.get_buffer_pointer(code))[:slang.get_buffer_size(code)])
```

Because of `#subtype`, procs written for a base interface also accept derived ones. For example, `slang.link` and `slang.get_entry_point_code` take `^Component_Type`, and you can pass a `^Module` to them directly.

`Global_Session`, `Session`, `Module`, and `Entry_Point` are private types. You hold them through `:=` type inference instead of naming them. `Blob` and `Component_Type` are public because you need to name them yourself, e.g. in helper signatures and in the components slice above.

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
- Add a public snake_case free-proc wrapper next to the vtable that takes the object as `this` and follows the return conventions above: out-pointers become named return values, then `diagnostics: ^Blob` if the call has one, then `ok: bool`. For example:

  ```odin
  load_module :: proc(this: ^Session, module_name: cstring) -> (module: ^Module, diagnostics: ^Blob, ok: bool) {
      module = this->LoadModule(module_name, &diagnostics)
      ok = module != nil
      return
  }
  ```

## License

This project's own code is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Slang itself is licensed under **Apache License 2.0 with LLVM exceptions** - see [`include/LICENSE-slang.txt`](include/LICENSE-slang.txt).

## Related Projects

- [Slang Official Repository](https://github.com/shader-slang/slang) - The Slang shading language
- [Odin Language](https://odin-lang.org/) - The Odin programming language
