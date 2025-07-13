@echo off
echo ==========================================
echo   DX11 Game Engine - Project Generator
echo ==========================================

REM Check if build directory exists, if not create it
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

REM Navigate to build directory
cd build

REM Generate Visual Studio project files
echo Generating Visual Studio 2022 project files...
cmake .. -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% == 0 (
    echo.
    echo ==========================================
    echo   Project generation completed successfully!
    echo ==========================================
    echo.
    echo You can now open the project in Visual Studio:
    echo   build\DX11GameEngine.sln
    echo.
    echo Or compile from command line:
    echo   cmake --build . --config Release
    echo ==========================================
) else (
    echo.
    echo ==========================================
    echo   ERROR: Project generation failed!
    echo ==========================================
    echo.
    echo Make sure you have:
    echo   - CMake installed and in PATH
    echo   - Visual Studio 2022 with C++ tools
    echo   - Windows SDK
    echo ==========================================
)

pause
