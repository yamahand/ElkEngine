@echo off
REM Scripts\build.bat

setlocal

set BUILD_TYPE=Debug
set COMPILER=VS2022
set ARCH=x64

REM マッピング: COMPILER -> CMake ジェネレータ名
if /I "%COMPILER%"=="VS2022" (
    set "GENERATOR=Visual Studio 17 2022"
) else if /I "%COMPILER%"=="VS2026" (
    set "GENERATOR=Visual Studio 18 2026"
) else (
    rem デフォルト: VS2022
    set "GENERATOR=Visual Studio 17 2022"
)

REM CMake configure (マルチコンフィグなので -DCMAKE_BUILD_TYPE は指定しない)
cmake -B build -G "%GENERATOR%" -A %ARCH% ^
      -DBUILD_EDITOR=ON

REM ビルド（Visual Studio 等のマルチコンフィグは --config を使う）
cmake --build build --config %BUILD_TYPE% --parallel

REM C#エディタビルド
if exist Editor (
    dotnet build Editor\ElkEditor.csproj -c %BUILD_TYPE%
    
    REM DLL コピー: CMakeで固定した出力ディレクトリからコピー
    set "DEST_DIR=Editor\bin\%BUILD_TYPE%\net9.0-windows\"
    if not exist "%DEST_DIR%" mkdir "%DEST_DIR%"
    
    REM C++ のランタイム出力は build\bin\<Config>\ にある想定
    if exist build\bin\%BUILD_TYPE%\ (
        copy /Y build\bin\%BUILD_TYPE%\*.dll "%DEST_DIR%"
    ) else (
        echo Warning: build\bin\%BUILD_TYPE% not found. Skip DLL copy.
    )
)

endlocal