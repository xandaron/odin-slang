# Odin Slang Bindings

[![License](https://img.shields.io/badge/license-MIT-blue)](#license)

## Overview

This project provides comprehensive Odin bindings for Slang, a shader programming language that extends HLSL with modern language features. The bindings are automatically generated using a custom C wrapper and bindgen configuration.

## What is Slang?

[Slang](https://github.com/shader-slang/slang) is a shading language that makes it easier to build and maintain large shader codebases. It provides:

- **Modularity** - Import/export system for sharing code across shaders
- **Generics** - Parametric polymorphism for reusable shader components  
- **Interfaces** - Abstract types for flexible shader architectures
- **Multiple Targets** - Compile to HLSL, GLSL, SPIR-V, Metal, CUDA, and more
- **Reflection** - Rich metadata about shader parameters and resources

## Quick Start

### Prerequisites

- [Odin compiler](https://odin-lang.org/) (latest version recommended)
- Visual Studio 2019+ (Windows) or GCC/Clang (Linux)
- CMake 3.15+

### Building

**Windows:**

```bash
./build.bat
bindgen .
```

**Linux:**

```bash
./build.sh
bindgen .
```

## Examples

The `demo/` directory contains examples of compiling slang files into glsl:

- **`demo/main.odin`** - Complete shader compilation pipeline
- **`demo/shaders/VertexDemo.slang`** - Vertex and fragment shader example
- **`demo/shaders/ComputeDemo.slang`** - Compute shader with shared memory

### Running the Demo

```bash
cd demo
odin build main.odin -out:test.exe
./test.exe
```

This will:

1. Compile Slang shaders to GLSL
2. Generate `vert.glsl`, `frag.glsl`, and `comp.glsl` output files
3. Demonstrate the complete compilation workflow

## Building from Source

### Generate New Bindings

If you need to regenerate the bindings (e.g., after updating Slang):

1. Update Slang libraries in `libs/` and headers in `include/`
2. Modify `bindgen.sjson` if needed
3. Run your bindgen tool with the configuration
4. The output will be generated in `odin-slang/`

### Custom C Wrapper

The `slangc.h` and `slangc.cpp` files provide a simplified C API that:

- Handles C++ exceptions and converts them to error codes
- Provides consistent memory management patterns  
- Simplifies complex Slang API patterns for easier binding generation

## Error Handling

The bindings use Slang's built-in error handling:

```odin
// Check for errors after operations that return pointers
module := slang.loadModule(session, "shader.slang", nil)
if module == nil {
    if slang.hasError() {
        errorMsg := slang.getLastErrorMessage()
        fmt.printf("Error: %s\n", errorMsg)
    }
}

// Or capture diagnostics explicitly
diagnostics: ^slang.Blob
module := slang.loadModule(session, "shader.slang", &diagnostics)
if diagnostics != nil {
    fmt.printf("Diagnostics: %s\n", (cstring)slang.getBlobData(diagnostics))
    slang.releaseBlob(diagnostics)
}
```

## Platform Support

- **Windows** - Full support with MSVC
- **Linux** - Should work but untested

Libraries are automatically selected based on the target platform in `import.odin`.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

The Slang compiler and libraries are licensed under the MIT License by the Slang team.

## Related Projects

- [Slang Official Repository](https://github.com/shader-slang/slang) - The Slang shading language
- [Odin Language](https://odin-lang.org/) - The Odin programming language
