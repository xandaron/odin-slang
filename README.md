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

- **`odin-slang/`** - the `slang` Odin package (the binding itself).
- **`include/`** - the Slang headers this binding was written against, plus Slang's own `LICENSE-slang.txt`. Kept for reference when extending the binding.
- **`demo/`** - a small end-to-end example: compiles a vertex/fragment `.slang` file and a compute `.slang` file down to GLSL.

## Using the COM-lite interfaces

Every Slang object derives from `ISlangUnknown`. In Odin, call methods with the `->` syntax and release objects explicitly when you're done with them:

```odin
import slang "odin-slang"

global_session: ^slang.IGlobalSession
slang.createGlobalSession(slang.API_VERSION, &global_session)
defer global_session->Release()

session_desc := slang.session_desc_default()
// ... fill in session_desc.targets / searchPaths ...

session: ^slang.ISession
global_session->CreateSession(&session_desc, &session)
defer session->Release()

diagnostics: ^slang.IBlob
module := session->LoadModule("shader.slang", &diagnostics)
if diagnostics != nil {
    fmt.eprintln(cstring(diagnostics->GetBufferPointer()))
    diagnostics->Release()
}
if module == nil {
    // handle failure
}
defer module->Release()
```

`IModule` and `IEntryPoint` are themselves `IComponentType`s (via Odin's `#subtype`), so they can be passed directly to `ISession.CreateCompositeComponentType`:

```odin
entry_point: ^slang.IEntryPoint
module->FindAndCheckEntryPoint("main", .COMPUTE, &entry_point, &diagnostics)
defer entry_point->Release()

components := []^slang.IComponentType{module, entry_point}
program: ^slang.IComponentType
session->CreateCompositeComponentType(raw_data(components), len(components), &program, &diagnostics)
defer program->Release()

linked: ^slang.IComponentType
program->Link(&linked, &diagnostics)
defer linked->Release()

code: ^slang.IBlob
linked->GetEntryPointCode(0, 0, &code, &diagnostics)
defer code->Release()
```

Most fallible calls return a `slang.Result`; check it with `slang.succeeded(result)` / `slang.failed(result)`. A handful of convenience calls (`LoadModule`, `SpecializeType`, ...) return the object pointer directly and are `nil` on failure instead - matching slang.h.

See `demo/main.odin` for a complete, runnable example.

## Building

### Prerequisites

- [Odin compiler](https://odin-lang.org/)
- Prebuilt Slang libraries for your platform (see below)

### Getting Slang's libraries

This repo does not vendor Slang's compiled binaries - `*.dll`/`*.lib`/`*.so`/`*.dylib` are gitignored, so you need to supply your own copy after every fresh checkout. Download a release from the [Slang releases page](https://github.com/shader-slang/slang/releases) (or build Slang yourself from the `slang/` submodule/checkout) for your platform, then:

- **Windows**: copy `slang.lib` next to the binding (`odin-slang/slang.lib`) so the Odin linker can find it, and copy `slang.dll` next to whatever executable links against the binding (e.g. `demo/`). Recent Slang releases split the compiler itself out into `slang-compiler.dll`, which `slang.dll` loads at runtime - copy that alongside it too. Add whichever of `slang-rt.dll`, `slang-glslang.dll`, `slang-glsl-module.dll`, `slang-llvm.dll` your target formats need (GLSL output needs `slang-glslang.dll` and `slang-glsl-module.dll`).
- **Linux/macOS**: copy `libslang.so`/`libslang.dylib` (and friends) somewhere your dynamic linker can find them (e.g. alongside the executable, or a directory on `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH`).

`odin-slang/` is where `foreign import` resolves the `.lib` from; the runtime shared libraries need to be next to (or discoverable by) the final built executable. Since none of this is tracked in git, re-copy these files after cloning the repo fresh.

### Running the demo

```bash
cd demo
odin build . -out:test.exe
./test.exe
```

This compiles `shaders/VertexDemo.slang` and `shaders/ComputeDemo.slang` to GLSL, writing `vert.glsl`, `frag.glsl`, and `comp.glsl`.

## Extending the binding

`odin-slang/` mirrors the subset of `include/slang.h` needed for a full module-load -> link -> emit-code pipeline (`IGlobalSession`, `ISession`, `IComponentType`, `IModule`, `IEntryPoint`, `IBlob`). Reflection types (`TypeReflection`, `TypeLayoutReflection`, `ProgramLayout`, `FunctionReflection`, `DeclReflection`) and a few peripheral interfaces (`ISlangFileSystem`, `ISlangSharedLibrary`, ...) are declared as opaque pointer types - the ABI shapes are reserved but their vtables aren't bound yet.

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
