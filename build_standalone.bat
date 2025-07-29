@echo off
echo Building Slang C Wrapper (Standalone)...

REM Create build directory
if not exist build mkdir build
cd build

REM Use a simpler CMakeLists that doesn't build Slang
cmake .. -G "Visual Studio 17 2022" -A x64 -DUSE_PREBUILT_SLANG=ON

REM Build the project
cmake --build . --config Release

REM Copy the built library to the root directory for easier access
if exist Release\slang_wrapper.dll copy Release\slang_wrapper.dll ..\slang_wrapper.dll
if exist Release\slang_wrapper.lib copy Release\slang_wrapper.lib ..\slang_wrapper.lib
