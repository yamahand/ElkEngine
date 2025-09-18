@echo off
REM ElkGameEngine Project Generation Script for Windows
REM Usage: generate.bat [clean|vs2019|vs2022|help]

setlocal

REM ========================================
REM 設定
REM ========================================

set PROJECT_NAME=ElkGameEngine
set BUILD_DIR=build
set DEFAULT_GENERATOR="Visual Studio 17 2022"
set ARCHITECTURE=x64

REM ========================================
REM 引数処理
REM ========================================

set GENERATOR=%DEFAULT_GENERATOR%
set CLEAN_BUILD=false
set OPEN_VS=false

if "%1"=="" goto generate
if "%1"=="help" goto show_help
if "%1"=="clean" set CLEAN_BUILD=true && shift && goto parse_args
if "%1"=="vs2019" set GENERATOR="Visual Studio 16 2019" && shift && goto parse_args
if "%1"=="vs2022" set GENERATOR="Visual Studio 17 2022" && shift && goto parse_args
if "%1"=="open" set OPEN_VS=true && goto open_solution

:parse_args
if "%1"=="vs2019" set GENERATOR="Visual Studio 16 2019"
if "%1"=="vs2022" set GENERATOR="Visual Studio 17 2022"
if "%1"=="open" set OPEN_VS=true
goto generate

:show_help
echo.
echo ElkGameEngine Project Generation Script
echo.
echo Usage: generate.bat [options]
echo.
echo Options:
echo   clean          Clean build directory before generating
echo   vs2019         Generate Visual Studio 2019 solution
echo   vs2022         Generate Visual Studio 2022 solution (default)
echo   open           Generate and open Visual Studio solution
echo   help           Show this help message
echo.
echo Examples:
echo   generate.bat                Generate VS2022 solution
echo   generate.bat clean vs2019   Clean and generate VS2019 solution  
echo   generate.bat open           Generate and open VS2022 solution
echo.
echo Note: This script only generates project files, does not build.
echo       Use build.bat to actually compile the project.
echo.
pause
goto end

:generate

REM ========================================
REM プロジェクト生成開始
REM ========================================

echo.
echo =========================================
echo  %PROJECT_NAME% Project Generation
echo =========================================
echo  Generator: %GENERATOR%
echo  Architecture: %ARCHITECTURE%
echo  Clean Build: %CLEAN_BUILD%
echo =========================================
echo.

REM CMakeバージョン確認
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ? CMake is not installed or not in PATH!
    echo.
    echo Please install CMake and make sure it's in your PATH.
    echo Download from: https://cmake.org/download/
    echo.
    pause
    goto error_exit
)

echo ? CMake found

REM Visual Studioインストール確認
if %GENERATOR%=="Visual Studio 17 2022" (
    where devenv >nul 2>&1
    if %errorlevel% neq 0 (
        echo ??  Visual Studio 2022 not found in PATH
        echo    This might still work if VS2022 is installed
    ) else (
        echo ? Visual Studio 2022 found
    )
) else (
    echo ? Using %GENERATOR%
)

echo.

REM クリーンビルド
if "%CLEAN_BUILD%"=="true" (
    echo ? Cleaning build directory...
    if exist %BUILD_DIR% (
        echo Removing existing build directory...
        rmdir /s /q %BUILD_DIR%
        if %errorlevel% equ 0 (
            echo ? Build directory cleaned successfully
        ) else (
            echo ? Failed to clean build directory
            echo Some files might be in use. Close Visual Studio and try again.
            pause
            goto error_exit
        )
    ) else (
        echo ??  Build directory does not exist, skipping clean
    )
    echo.
)

REM ビルドディレクトリ作成
if not exist %BUILD_DIR% (
    echo ? Creating build directory...
    mkdir %BUILD_DIR%
    if %errorlevel% neq 0 (
        echo ? Failed to create build directory
        pause
        goto error_exit
    )
    echo ? Build directory created
    echo.
)

cd %BUILD_DIR%

REM ========================================
REM CMakeプロジェクト生成
REM ========================================

echo ? Generating project files with CMake...
echo.
echo Generator: %GENERATOR%
echo Target Architecture: %ARCHITECTURE%
echo.

cmake .. ^
    -G %GENERATOR% ^
    -A %ARCHITECTURE% ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DBUILD_EDITOR=OFF ^
    -DBUILD_TOOLS=OFF ^
    -DBUILD_SAMPLES=OFF ^
    -DELK_ENABLE_LOGGING=ON ^
    -DELK_ENABLE_PROFILER=ON ^
    -DELK_USE_SPDLOG=ON ^
    -DELK_FETCHCONTENT_AUTO=ON

if %errorlevel% neq 0 (
    echo.
    echo ? CMake project generation failed!
    echo.
    echo Common causes:
    echo  - CMake version too old (requires 3.20+)
    echo  - Visual Studio not properly installed
    echo  - Missing C++ development tools
    echo  - Network issues downloading dependencies
    echo.
    echo Solutions:
    echo  1. Update CMake to latest version
    echo  2. Install Visual Studio with C++ development tools
    echo  3. Check internet connection for FetchContent dependencies
    echo.
    pause
    goto error_exit
)

echo.
echo ? Project files generated successfully!
echo.

REM ========================================
REM 生成結果の確認
REM ========================================

echo ? Checking generated files...
echo.

if exist "%PROJECT_NAME%.sln" (
    echo ? Solution file: %PROJECT_NAME%.sln
) else (
    echo ? Solution file not found!
    goto error_exit
)

REM プロジェクトファイルの確認
set PROJECT_FOUND=false
if exist "ElkEngine\ElkEngine.vcxproj" (
    echo ? ElkEngine project: ElkEngine\ElkEngine.vcxproj
    set PROJECT_FOUND=true
)

if exist "Runtime\Runtime.vcxproj" (
    echo ? Runtime project: Runtime\Runtime.vcxproj
    set PROJECT_FOUND=true
)

if exist "Editor\Editor.vcxproj" (
    echo ? Editor project: Editor\Editor.vcxproj
    set PROJECT_FOUND=true
)

if "%PROJECT_FOUND%"=="false" (
    echo ? No project files found!
    goto error_exit
)

REM CMakeファイルの確認
if exist "CMakeCache.txt" (
    echo ? CMake cache: CMakeCache.txt
)

echo.

REM ========================================
REM 完了メッセージ
REM ========================================

echo =========================================
echo  Project Generation Complete! 
echo =========================================
echo.
echo ? Generated files location: %BUILD_DIR%\
echo ? Solution file: %PROJECT_NAME%.sln
echo.
echo ? Next steps:
echo   1. Open %PROJECT_NAME%.sln in Visual Studio
echo   2. Set Runtime as startup project (if not already)
echo   3. Build and run the project (F5)
echo.
echo ? Quick commands:
echo   generate.bat open     - Generate and open solution
echo   build.bat            - Build the project
echo   build.bat open       - Open existing solution
echo.
echo =========================================

REM Visual Studioで開く（オプション）
if "%OPEN_VS%"=="true" (
    goto open_solution
) else (
    echo.
    echo To open in Visual Studio, run: generate.bat open
    echo.
)

cd ..
goto end

REM ========================================
REM Visual Studio で開く
REM ========================================

:open_solution
if not exist %BUILD_DIR% (
    echo ? Build directory does not exist. Please run generate first.
    pause
    goto end
)

cd %BUILD_DIR%

if exist "%PROJECT_NAME%.sln" (
    echo ? Opening Visual Studio...
    start "" "%PROJECT_NAME%.sln"
    echo.
    echo ? Visual Studio should open shortly
    echo.
    echo ? Tips for first-time setup:
    echo   - Set Runtime as startup project (right-click → Set as Startup Project)
    echo   - Use Debug configuration for development
    echo   - Use Release configuration for final builds
    echo.
) else (
    echo ? Solution file not found. Please run generation first.
    echo.
    echo Run: generate.bat
    echo.
    pause
)

cd ..
goto end

REM ========================================
REM エラー終了
REM ========================================

:error_exit
cd ..
echo.
echo ? Project generation failed!
echo.
echo ? Troubleshooting steps:
echo   1. Make sure CMake 3.20+ is installed
echo   2. Verify Visual Studio 2019/2022 with C++ tools
echo   3. Check internet connection (for dependencies)
echo   4. Try: generate.bat clean
echo   5. Check CMakeLists.txt syntax
echo.
echo ? For help, check:
echo   - README.md
echo   - GitHub Issues
echo   - Project documentation
echo.
pause
exit /b 1

:end
endlocal