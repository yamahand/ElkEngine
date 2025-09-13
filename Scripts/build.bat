@echo off
REM ElkGameEngine Build Script for Windows
REM Usage: build.bat [clean|debug|release|install|help]

setlocal

REM ========================================
REM 設定
REM ========================================

set PROJECT_NAME=ElkGameEngine
set BUILD_DIR=build
set GENERATOR="Visual Studio 17 2022"
set ARCHITECTURE=x64
set JOBS=%NUMBER_OF_PROCESSORS%

REM ========================================
REM 引数処理
REM ========================================

set BUILD_TYPE=Debug
set CLEAN_BUILD=false
set INSTALL_BUILD=false
set OPEN_VS=false

if "%1"=="" goto build
if "%1"=="help" goto show_help
if "%1"=="clean" set CLEAN_BUILD=true && shift && goto parse_args
if "%1"=="debug" set BUILD_TYPE=Debug && shift && goto parse_args
if "%1"=="release" set BUILD_TYPE=Release && shift && goto parse_args
if "%1"=="install" set INSTALL_BUILD=true && shift && goto parse_args
if "%1"=="open" set OPEN_VS=true && goto open_visual_studio

:parse_args
if "%1"=="install" set INSTALL_BUILD=true
goto build

:show_help
echo.
echo ElkGameEngine Build Script
echo.
echo Usage: build.bat [options]
echo.
echo Options:
echo   clean          Clean build directory before building
echo   debug          Build Debug configuration (default)
echo   release        Build Release configuration
echo   install        Build and install to install directory
echo   open           Open Visual Studio solution
echo   help           Show this help message
echo.
echo Examples:
echo   build.bat                    Build Debug configuration
echo   build.bat clean release      Clean build and build Release
echo   build.bat release install    Build Release and install
echo.
pause
goto end

:build

REM ========================================
REM ビルド開始
REM ========================================

echo.
echo =========================================
echo  %PROJECT_NAME% Build Script
echo =========================================
echo  Build Type: %BUILD_TYPE%
echo  Clean Build: %CLEAN_BUILD%
echo  Install: %INSTALL_BUILD%
echo  Generator: %GENERATOR%
echo  Architecture: %ARCHITECTURE%
echo =========================================
echo.

REM クリーンビルド
if "%CLEAN_BUILD%"=="true" (
    echo Cleaning build directory...
    if exist %BUILD_DIR% (
        rmdir /s /q %BUILD_DIR%
        echo Build directory cleaned.
    ) else (
        echo Build directory does not exist, skipping clean.
    )
    echo.
)

REM ビルドディレクトリ作成
if not exist %BUILD_DIR% (
    echo Creating build directory...
    mkdir %BUILD_DIR%
)

cd %BUILD_DIR%

REM ========================================
REM CMake設定
REM ========================================

echo Configuring with CMake...
echo.

cmake .. ^
    -G %GENERATOR% ^
    -A %ARCHITECTURE% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DBUILD_EDITOR=ON ^
    -DBUILD_TOOLS=OFF ^
    -DBUILD_SAMPLES=OFF ^
    -DELK_ENABLE_LOGGING=ON ^
    -DELK_ENABLE_PROFILER=ON

if %errorlevel% neq 0 (
    echo.
    echo ? CMake configuration failed!
    echo.
    pause
    goto error_exit
)

pause
exit /b 0

echo.
echo ? CMake configuration completed successfully!
echo.

REM ========================================
REM ビルド実行
REM ========================================

echo Building %BUILD_TYPE% configuration...
echo.

cmake --build . ^
    --config %BUILD_TYPE% ^
    --parallel %JOBS% ^
    --verbose

if %errorlevel% neq 0 (
    echo.
    echo ? Build failed!
    echo.
    pause
    goto error_exit
)

echo.
echo ? Build completed successfully!
echo.

REM ========================================
REM インストール（オプション）
REM ========================================

if "%INSTALL_BUILD%"=="true" (
    echo Installing...
    echo.
    
    cmake --install . --config %BUILD_TYPE%
    
    if %errorlevel% neq 0 (
        echo.
        echo ? Install failed!
        echo.
        pause
        goto error_exit
    )
    
    echo.
    echo ? Install completed successfully!
    echo.
)

REM ========================================
REM 完了メッセージ
REM ========================================

echo.
echo =========================================
echo  Build Summary
echo =========================================
echo  Project: %PROJECT_NAME%
echo  Configuration: %BUILD_TYPE%
echo  Status: SUCCESS
echo.

if exist "%BUILD_TYPE%\bin\Runtime.exe" (
    echo  Runtime executable: %BUILD_TYPE%\bin\Runtime.exe
)

if exist "%BUILD_TYPE%\bin\Editor.exe" (
    echo  Editor executable: %BUILD_TYPE%\bin\Editor.exe
)

echo.
echo  To run the application:
echo    cd %BUILD_DIR%\%BUILD_TYPE%\bin
echo    Runtime.exe
echo.
echo  To open in Visual Studio:
echo    build.bat open
echo.
echo =========================================

cd ..
goto end

REM ========================================
REM Visual Studio を開く
REM ========================================

:open_visual_studio
if not exist %BUILD_DIR% (
    echo Build directory does not exist. Please run build first.
    pause
    goto end
)

if exist "%BUILD_DIR%\%PROJECT_NAME%.sln" (
    echo Opening Visual Studio...
    start "" "%BUILD_DIR%\%PROJECT_NAME%.sln"
) else (
    echo Solution file not found. Please run build first.
    pause
)
goto end

REM ========================================
REM エラー終了
REM ========================================

:error_exit
cd ..
echo.
echo Build process failed. Check the error messages above.
echo.
echo Common solutions:
echo  - Make sure Visual Studio 2022 is installed
echo  - Check that CMake is in your PATH
echo  - Verify all dependencies are available
echo  - Try running 'build.bat clean' first
echo.
pause
exit /b 1

:end
endlocal