@echo off
setlocal

echo Cleaning and building 32-bit core library...
if exist build32 (
    rmdir /s /q build32
    echo Removed build32 directory
)

rem The DPD vcpkg feature is x64-only, so Win32 intentionally tests the core library.
cmake -S . -B build32 -A Win32 ^
    -DCMAKE_INSTALL_PREFIX=installed32 ^
    -DENABLE_DPD=OFF ^
    -DENABLE_BUILDING_WRAPPERS=OFF ^
    -DENABLE_DOCUMENTATION=OFF
if %ERRORLEVEL% neq 0 (
    echo CMake configuration for 32-bit failed
    exit /b %ERRORLEVEL%
)

cmake --build build32 --config Release --parallel
if %ERRORLEVEL% neq 0 (
    echo CMake build for 32-bit failed
    exit /b %ERRORLEVEL%
)

cmake --install build32 --config Release
if %ERRORLEVEL% neq 0 (
    echo CMake install for 32-bit failed
    exit /b %ERRORLEVEL%
)

echo.
echo Cleaning and building 64-bit library...
if exist build64 (
    rmdir /s /q build64
    echo Removed build64 directory
)

if defined VCPKG_ROOT (
    if not exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
        echo VCPKG_ROOT is set but the vcpkg toolchain file was not found.
        exit /b 1
    )

    cmake -S . -B build64 -A x64 ^
        -DCMAKE_INSTALL_PREFIX=installed64 ^
        -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
        -DVCPKG_TARGET_TRIPLET=x64-windows ^
        -DENABLE_DPD=ON ^
        -DENABLE_BUILDING_WRAPPERS=OFF ^
        -DENABLE_DOCUMENTATION=OFF
) else (
    echo VCPKG_ROOT is not set. The 64-bit build will be core-only ^(ENABLE_DPD=OFF^).
    cmake -S . -B build64 -A x64 ^
        -DCMAKE_INSTALL_PREFIX=installed64 ^
        -DENABLE_DPD=OFF ^
        -DENABLE_BUILDING_WRAPPERS=OFF ^
        -DENABLE_DOCUMENTATION=OFF
)

if %ERRORLEVEL% neq 0 (
    echo CMake configuration for 64-bit failed
    exit /b %ERRORLEVEL%
)

cmake --build build64 --config Release --parallel
if %ERRORLEVEL% neq 0 (
    echo CMake build for 64-bit failed
    exit /b %ERRORLEVEL%
)

cmake --install build64 --config Release
if %ERRORLEVEL% neq 0 (
    echo CMake install for 64-bit failed
    exit /b %ERRORLEVEL%
)

echo.
echo Build completed successfully
exit /b 0
