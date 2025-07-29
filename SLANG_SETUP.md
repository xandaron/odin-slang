# Getting Slang for the C Wrapper

The C wrapper needs the Slang library to work. You have several options:

## Option 1: Download Prebuilt Slang (Recommended)

1. **Download**: Go to https://github.com/shader-slang/slang/releases
2. **Get the latest**: Download the Windows x64 release (e.g., `slang-2025.13.1-windows-x86_64.zip`)
3. **Extract**: Extract to a `slang/` folder in your project directory

Expected structure after extraction:
```
odin-slang/
├── slang/
│   ├── bin/
│   │   ├── slang.exe
│   │   ├── slang.dll
│   │   └── ...
│   ├── lib/
│   │   ├── slang.lib
│   │   └── ...
│   └── include/
│       ├── slang.h
│       ├── slang-com-ptr.h
│       └── ...
├── slangc.h
├── slangc.cpp
└── build_standalone.bat
```

4. **Build**: Run `build_standalone.bat`

## Option 2: Build Slang from Source

1. **Clone with submodules**:
   ```cmd
   cd slang
   git submodule update --init --recursive
   ```

2. **Build Slang**:
   ```cmd
   cd slang
   cmake -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release
   ```

3. **Build wrapper**: Run `build.bat`

## Option 3: Use Package Manager

If you have vcpkg:
```cmd
vcpkg install slang
```

Then modify the CMakeLists.txt to use `find_package(slang CONFIG REQUIRED)`.

## Troubleshooting

- **"slang.exe not found"**: Make sure you extracted to the right location
- **"slang.lib not found"**: Check that you downloaded the correct architecture (x64)
- **CMake errors**: Try the standalone build approach with prebuilt binaries

## Testing the Installation

After building, test with:
```cmd
build\Release\slangc_spirv_example.exe
```

This should compile a simple compute shader to SPIRV and save it as `compute.spv`.
