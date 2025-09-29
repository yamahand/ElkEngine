# ElkGameEngine Project Generation Script for Windows (PowerShell)
# Usage: .\generate.ps1 [clean|vs2019|vs2022|open|help]


param(
    [string[]]$Arguments
)

# ========================================
# 設定
# ========================================
$PROJECT_NAME = "ElkGameEngine"
$BUILD_DIR = "build"
$DEFAULT_GENERATOR = "Visual Studio 17 2022"
$ARCHITECTURE = "x64"

# ========================================
# 引数処理
# ========================================
$GENERATOR = $DEFAULT_GENERATOR
$CLEAN_BUILD = $false
$OPEN_VS = $false

function Show-Help {
    Write-Host "\nElkGameEngine Project Generation Script"
    Write-Host "\nUsage: generate.ps1 [options]"
    Write-Host "\nOptions:"
    Write-Host "  clean          Clean build directory before generating"
    Write-Host "  vs2019         Generate Visual Studio 2019 solution"
    Write-Host "  vs2022         Generate Visual Studio 2022 solution (default)"
    Write-Host "  open           Generate and open Visual Studio solution"
    Write-Host "  help           Show this help message"
    Write-Host "\nExamples:"
    Write-Host "  .\generate.ps1                Generate VS2022 solution"
    Write-Host "  .\generate.ps1 clean vs2019   Clean and generate VS2019 solution"
    Write-Host "  .\generate.ps1 open           Generate and open VS2022 solution"
    Write-Host "\nNote: This script only generates project files, does not build."
    Write-Host "      Use build.ps1 to actually compile the project.\n"
}

foreach ($arg in $Arguments) {
    switch ($arg.ToLower()) {
        "help"   { Show-Help; return }
        "clean"  { $CLEAN_BUILD = $true }
        "vs2019" { $GENERATOR = "Visual Studio 16 2019" }
        "vs2022" { $GENERATOR = "Visual Studio 17 2022" }
        "open"   { $OPEN_VS = $true }
    }
}

# ========================================
# プロジェクト生成開始
# ========================================
$LAST_STEP = ""
$LAST_ERR = ""

Write-Host "\n========================================="
Write-Host " $PROJECT_NAME Project Generation"
Write-Host "========================================="
Write-Host " Generator: $GENERATOR"
Write-Host " Architecture: $ARCHITECTURE"
Write-Host " Clean Build: $CLEAN_BUILD"
Write-Host "========================================="
Write-Host ""

# CMakeバージョン確認
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "? CMake is not installed or not in PATH!"
    Write-Host "\nPlease install CMake and make sure it's in your PATH."
    Write-Host "Download from: https://cmake.org/download/\n"
    $LAST_STEP = "CMakeNotFound"
    exit 1
}
Write-Host "? CMake found"

# Visual Studioインストール確認
if ($GENERATOR -eq "Visual Studio 17 2022") {
    if (-not (Get-Command devenv.exe -ErrorAction SilentlyContinue)) {
        Write-Host "??  Visual Studio 2022 not found in PATH"
        Write-Host "    This might still work if VS2022 is installed"
    } else {
        Write-Host "? Visual Studio 2022 found"
    }
} else {
    Write-Host "? Using $GENERATOR"
}
Write-Host ""

# クリーンビルド
if ($CLEAN_BUILD) {
    Write-Host "? Cleaning build directory..."
    if (Test-Path $BUILD_DIR) {
        try {
            Remove-Item -Recurse -Force $BUILD_DIR
            Write-Host "? Build directory cleaned successfully"
        } catch {
            Write-Host "? Failed to clean build directory: $_"
            $LAST_STEP = "CleanRmdirFailed"
            $LAST_ERR = $_
            exit 1
        }
    } else {
        Write-Host "??  Build directory does not exist, skipping clean"
    }
    Write-Host ""
}

# ビルドディレクトリ作成
if (-not (Test-Path $BUILD_DIR)) {
    Write-Host "? Creating build directory..."
    try {
        New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
        Write-Host "? Build directory created successfully"
    } catch {
        Write-Host "? Failed to create build directory: $_"
        $LAST_STEP = "MkdirFailed"
        $LAST_ERR = $_
        exit 1
    }
    Write-Host ""
}

Push-Location $BUILD_DIR

# ========================================
# CMakeプロジェクト生成
# ========================================
Write-Host "? Generating project files with CMake...\n"
Write-Host "Generator: $GENERATOR"
Write-Host "Target Architecture: $ARCHITECTURE\n"

$cmakeArgs = @(
    "..",
    "-G", "$GENERATOR",
    "-A", "$ARCHITECTURE",
    "-DCMAKE_BUILD_TYPE=Debug",
    "-DBUILD_SHARED_LIBS=OFF",
    "-DBUILD_EDITOR=OFF",
    "-DBUILD_TOOLS=OFF",
    "-DBUILD_SAMPLES=OFF",
    "-DELK_ENABLE_LOGGING=ON",
    "-DELK_ENABLE_PROFILER=ON",
    "-DELK_USE_SPDLOG=ON"
)

$cmakeResult = & cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host "\n? CMake project generation failed!\n"
    Write-Host "Common causes:"
    Write-Host " - CMake version too old (requires 3.20+)"
    Write-Host " - Visual Studio not properly installed"
    Write-Host " - Missing C++ development tools"
    Write-Host " - Network issues downloading dependencies"
    Write-Host "Solutions:"
    Write-Host " 1. Update CMake to latest version"
    Write-Host " 2. Install Visual Studio with C++ development tools"
    Write-Host " 3. Check internet connection for FetchContent dependencies"
    $LAST_STEP = "CMakeFailed"
    $LAST_ERR = $LASTEXITCODE
    Pop-Location
    exit 1
}

Write-Host "\n? Project files generated successfully!\n"

# ========================================
# 生成結果の確認
# ========================================
Write-Host "? Checking generated files...\n"

if (Test-Path "$PROJECT_NAME.sln") {
    Write-Host "? Solution file: $PROJECT_NAME.sln"
} else {
    Write-Host "? Solution file not found!"
    $LAST_STEP = "SolutionMissing"
    Pop-Location
    exit 1
}

$PROJECT_FOUND = $false
if (Test-Path "ElkEngine\ElkEngine.vcxproj") {
    Write-Host "? ElkEngine project: ElkEngine\ElkEngine.vcxproj"
    $PROJECT_FOUND = $true
}
if (Test-Path "Runtime\Runtime.vcxproj") {
    Write-Host "? Runtime project: Runtime\Runtime.vcxproj"
    $PROJECT_FOUND = $true
}
if (Test-Path "Editor\Editor.vcxproj") {
    Write-Host "? Editor project: Editor\Editor.vcxproj"
    $PROJECT_FOUND = $true
}
if (-not $PROJECT_FOUND) {
    Write-Host "? No project files found!"
    $LAST_STEP = "ProjectsMissing"
    Pop-Location
    exit 1
}

if (Test-Path "CMakeCache.txt") {
    Write-Host "? CMake cache: CMakeCache.txt"
}
Write-Host ""

# ========================================
# 完了メッセージ
# ========================================
Write-Host "========================================="
Write-Host " Project Generation Complete! "
Write-Host "========================================="
Write-Host ""
Write-Host "? Generated files location: $BUILD_DIR\"
Write-Host "? Solution file: $PROJECT_NAME.sln"
Write-Host ""
Write-Host "? Next steps:"
Write-Host "  1. Open $PROJECT_NAME.sln in Visual Studio"
Write-Host "  2. Set Runtime as startup project (if not already)"
Write-Host "  3. Build and run the project (F5)"
Write-Host ""
Write-Host "? Quick commands:"
Write-Host "  .\generate.ps1 open     - Generate and open solution"
Write-Host "  .\build.ps1            - Build the project"
Write-Host "  .\build.ps1 open       - Open existing solution"
Write-Host ""
Write-Host "========================================="

# Visual Studioで開く（オプション）
if ($OPEN_VS) {
    if (-not (Test-Path "$PROJECT_NAME.sln")) {
        Write-Host "? Solution file not found. Please run generation first."
        Pop-Location
        exit 1
    }
    Write-Host "? Opening Visual Studio..."
    Start-Process "$PROJECT_NAME.sln"
    Write-Host "\n? Visual Studio should open shortly\n"
    Write-Host "? Tips for first-time setup:"
    Write-Host "  - Set Runtime as startup project (right-click → Set as Startup Project)"
    Write-Host "  - Use Debug configuration for development"
    Write-Host "  - Use Release configuration for final builds\n"
}

Pop-Location
