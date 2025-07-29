@echo off
echo Building Slang C Wrapper...

REM Check if we have Slang source
if not exist slang\CMakeLists.txt (
    echo ERROR: Slang source not found!
    echo.
    echo Please either:
    echo   1. Use build_standalone.bat with prebuilt Slang binaries
    echo   2. Or ensure the slang submodule is properly initialized
    echo.
    echo For option 2:
    echo   git submodule update --init --recursive
    echo.
    pause
    exit /b 1
)

REM Initialize git submodules if they don't exist
if not exist slang\external\miniz\CMakeLists.txt (
    echo Initializing git submodules...
    cd slang
    git submodule update --init --recursive
    if errorlevel 1 (
        cd ..
        echo.
        echo ERROR: Failed to initialize submodules.
        echo Try using build_standalone.bat instead.
        echo.
        pause
        exit /b 1
    )
    cd ..
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    echo Try using build_standalone.bat with prebuilt Slang binaries.
    pause
    exit /b 1
)

REM Build the project
echo Building...
cmake --build . --config Release
if errorlevel 1 (
    echo.
    echo Build failed!
    pause
    exit /b 1
)

REM Copy the built library to the root directory for easier access
if exist Release\slang_wrapper.dll copy Release\slang_wrapper.dll ..\slang_wrapper.dll
if exist Release\slang_wrapper.lib copy Release\slang_wrapper.lib ..\slang_wrapper.lib

echo.
echo Build complete!
echo.
echo Next steps:
echo 1. Run your bindgen tool to generate Odin bindings from slangc.h
echo 2. Link against slang_wrapper.dll when using the bindings
echo.
pause
