@echo off
REM Scripts\build.bat

setlocal

set BUILD_TYPE=Debug
set COMPILER=VS2022
set ARCH=x64

REM �}�b�s���O: COMPILER -> CMake �W�F�l���[�^��
if /I "%COMPILER%"=="VS2022" (
    set "GENERATOR=Visual Studio 17 2022"
) else if /I "%COMPILER%"=="VS2026" (
    set "GENERATOR=Visual Studio 18 2026"
) else (
    rem �f�t�H���g: VS2022
    set "GENERATOR=Visual Studio 17 2022"
)

REM CMake configure (�}���`�R���t�B�O�Ȃ̂� -DCMAKE_BUILD_TYPE �͎w�肵�Ȃ�)
cmake -B build -G "%GENERATOR%" -A %ARCH% ^
      -DBUILD_EDITOR=ON

REM �r���h�iVisual Studio ���̃}���`�R���t�B�O�� --config ���g���j
cmake --build build --config %BUILD_TYPE% --parallel

REM C#�G�f�B�^�r���h
if exist Editor (
    dotnet build Editor\ElkEditor.csproj -c %BUILD_TYPE%
    
    REM DLL �R�s�[: CMake�ŌŒ肵���o�̓f�B���N�g������R�s�[
    set "DEST_DIR=Editor\bin\%BUILD_TYPE%\net9.0-windows\"
    if not exist "%DEST_DIR%" mkdir "%DEST_DIR%"
    
    REM C++ �̃����^�C���o�͂� build\bin\<Config>\ �ɂ���z��
    if exist build\bin\%BUILD_TYPE%\ (
        copy /Y build\bin\%BUILD_TYPE%\*.dll "%DEST_DIR%"
    ) else (
        echo Warning: build\bin\%BUILD_TYPE% not found. Skip DLL copy.
    )
)

endlocal