@echo off
REM Simple CMake build script for Windows

setlocal EnableDelayedExpansion

where /Q cl.exe || (
	set __VSCMD_ARG_NO_LOGO=1
	for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set VS=%%i
	if "!VS!" equ "" (
		echo ERROR: MSVC installation not found
		exit /b 1
	)
	call "!VS!\Common7\Tools\vsdevcmd.bat" -arch=x64 -host_arch=x64 || exit /b 1
)

if "%VSCMD_ARG_TGT_ARCH%" neq "x64" (
	if "%ODIN_IGNORE_MSVC_CHECK%" == "" (
		echo ERROR: please run this from MSVC x64 native tools command prompt, 32-bit target is not supported!
		exit /b 1
	)
)

REM Parse arguments
set BUILD_TYPE=Release
set GENERATOR=

:parse_args
if "%~1"=="" goto :build
if /i "%~1"=="debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
if /i "%~1"=="ninja" (
    set GENERATOR=-G "Ninja"
    shift
    goto :parse_args
)
if /i "%~1"=="clean" (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
    echo Clean complete.
    goto :end
)
if /i "%~1"=="help" (
    echo Usage: cmake-build.bat [debug] [ninja] [clean] [help]
    echo.
    echo Options:
    echo   debug  - Build in Debug mode (default: Release)
    echo   ninja  - Use Ninja generator (default: Visual Studio)
    echo   clean  - Clean build directory
    echo   help   - Show this help
    echo.
    echo Examples:
    echo   cmake-build.bat          - Build Release with default generator
    echo   cmake-build.bat debug    - Build Debug
    echo   cmake-build.bat ninja    - Build with Ninja generator
    goto :end
)
shift
goto :parse_args

:build
REM Create build directory
if not exist build mkdir build

REM Configure
echo.
echo Configuring with CMake...
cd build
cmake .. %GENERATOR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    cd ..
    goto :error
)

REM Build
echo.
echo Building...
cmake --build . --config %BUILD_TYPE% --parallel
if %errorlevel% neq 0 (
    echo Build failed!
    cd ..
    goto :error
)

cd ..

echo.
echo Build completed successfully!
echo Output files are in: build\lib\ and build\bin\

REM Copy to bin directory for consistency
if not exist bin mkdir bin
if exist build\lib\Release\*.dll copy build\lib\Release\*.dll bin\ >nul
if exist build\lib\Release\*.lib copy build\lib\Release\*.lib bin\ >nul
if exist build\lib\Debug\*.dll copy build\lib\Debug\*.dll bin\ >nul
if exist build\lib\Debug\*.lib copy build\lib\Debug\*.lib bin\ >nul
if exist build\lib\*.dll copy build\lib\*.dll bin\ >nul
if exist build\lib\*.lib copy build\lib\*.lib bin\ >nul
if exist build\lib\*.a copy build\lib\*.a bin\ >nul

goto :end

:error
echo Build failed!
exit /b 1

:end
endlocal
