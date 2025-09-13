#!/bin/bash
# ElkGameEngine Build Script for Linux/macOS
# Usage: ./build.sh [clean|debug|release|install|help]

set -e  # Exit on any error

# ========================================
# 設定
# ========================================

PROJECT_NAME="ElkGameEngine"
BUILD_DIR="build"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo "4")

# ========================================
# カラー出力設定
# ========================================

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# ヘルパー関数
print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

# ========================================
# 引数処理
# ========================================

BUILD_TYPE="Debug"
CLEAN_BUILD=false
INSTALL_BUILD=false
VERBOSE=false

show_help() {
    echo
    echo "ElkGameEngine Build Script"
    echo
    echo "Usage: ./build.sh [options]"
    echo
    echo "Options:"
    echo "  clean          Clean build directory before building"
    echo "  debug          Build Debug configuration (default)"
    echo "  release        Build Release configuration"
    echo "  install        Build and install to install directory"
    echo "  verbose        Enable verbose build output"
    echo "  help           Show this help message"
    echo
    echo "Examples:"
    echo "  ./build.sh                     Build Debug configuration"
    echo "  ./build.sh clean release       Clean build and build Release"
    echo "  ./build.sh release install     Build Release and install"
    echo
}

# 引数解析
while [[ $# -gt 0 ]]; do
    case $1 in
        clean)
            CLEAN_BUILD=true
            shift
            ;;
        debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        release)
            BUILD_TYPE="Release"
            shift
            ;;
        install)
            INSTALL_BUILD=true
            shift
            ;;
        verbose)
            VERBOSE=true
            shift
            ;;
        help)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# ========================================
# システム情報取得
# ========================================

detect_system() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        PLATFORM="Linux"
        GENERATOR="Unix Makefiles"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        PLATFORM="macOS"
        GENERATOR="Unix Makefiles"
        # Xcodeが利用可能かチェック
        if command -v xcodebuild &> /dev/null; then
            GENERATOR="Xcode"
        fi
    else
        print_error "Unsupported operating system: $OSTYPE"
        exit 1
    fi
}

# 必要なツールの確認
check_dependencies() {
    print_info "Checking dependencies..."
    
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is not installed or not in PATH"
        exit 1
    fi
    
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    print_info "Found CMake version: $CMAKE_VERSION"
    
    if ! command -v git &> /dev/null; then
        print_warning "Git is not installed (may be needed for dependencies)"
    fi
    
    # コンパイラーチェック
    if command -v gcc &> /dev/null; then
        GCC_VERSION=$(gcc --version | head -n1)
        print_info "Found GCC: $GCC_VERSION"
    elif command -v clang &> /dev/null; then
        CLANG_VERSION=$(clang --version | head -n1)
        print_info "Found Clang: $CLANG_VERSION"
    else
        print_error "No suitable C++ compiler found (GCC or Clang required)"
        exit 1
    fi
    
    print_success "All dependencies satisfied"
    echo
}

# ========================================
# ビルド処理
# ========================================

main() {
    detect_system
    
    echo
    echo "========================================="
    echo "  $PROJECT_NAME Build Script"
    echo "========================================="
    echo "  Platform: $PLATFORM"
    echo "  Build Type: $BUILD_TYPE"
    echo "  Clean Build: $CLEAN_BUILD"
    echo "  Install: $INSTALL_BUILD"
    echo "  Generator: $GENERATOR"
    echo "  Jobs: $JOBS"
    echo "========================================="
    echo
    
    check_dependencies
    
    # クリーンビルド
    if [ "$CLEAN_BUILD" = true ]; then
        print_info "Cleaning build directory..."
        if [ -d "$BUILD_DIR" ]; then
            rm -rf "$BUILD_DIR"
            print_success "Build directory cleaned"
        else
            print_info "Build directory does not exist, skipping clean"
        fi
        echo
    fi
    
    # ビルドディレクトリ作成
    if [ ! -d "$BUILD_DIR" ]; then
        print_info "Creating build directory..."
        mkdir -p "$BUILD_DIR"
    fi
    
    cd "$BUILD_DIR"
    
    # ========================================
    # CMake設定
    # ========================================
    
    print_info "Configuring with CMake..."
    echo
    
    CMAKE_ARGS=(
        ".."
        "-G" "$GENERATOR"
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DBUILD_SHARED_LIBS=OFF"
        "-DBUILD_EDITOR=ON"
        "-DBUILD_TOOLS=ON"
        "-DBUILD_SAMPLES=OFF"
        "-DELK_ENABLE_LOGGING=ON"
        "-DELK_ENABLE_PROFILER=ON"
    )
    
    # プラットフォーム固有の設定
    if [ "$PLATFORM" = "macOS" ]; then
        CMAKE_ARGS+=(
            "-DCMAKE_OSX_ARCHITECTURES=x86_64"
            "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15"
        )
    fi
    
    if [ "$VERBOSE" = true ]; then
        CMAKE_ARGS+=("--verbose")
    fi
    
    if ! cmake "${CMAKE_ARGS[@]}"; then
        print_error "CMake configuration failed!"
        exit 1
    fi
    
    print_success "CMake configuration completed successfully!"
    echo
    
    # ========================================
    # ビルド実行
    # ========================================
    
    print_info "Building $BUILD_TYPE configuration..."
    echo
    
    BUILD_ARGS=(
        "--build" "."
        "--config" "$BUILD_TYPE"
        "--parallel" "$JOBS"
    )
    
    if [ "$VERBOSE" = true ]; then
        BUILD_ARGS+=("--verbose")
    fi
    
    if ! cmake "${BUILD_ARGS[@]}"; then
        print_error "Build failed!"
        exit 1
    fi
    
    print_success "Build completed successfully!"
    echo
    
    # ========================================
    # インストール（オプション）
    # ========================================
    
    if [ "$INSTALL_BUILD" = true ]; then
        print_info "Installing..."
        echo
        
        if ! cmake --install . --config "$BUILD_TYPE"; then
            print_error "Install failed!"
            exit 1
        fi
        
        print_success "Install completed successfully!"
        echo
    fi
    
    # ========================================
    # 完了メッセージ
    # ========================================
    
    echo
    echo "========================================="
    echo "  Build Summary"
    echo "========================================="
    echo "  Project: $PROJECT_NAME"
    echo "  Platform: $PLATFORM"
    echo "  Configuration: $BUILD_TYPE"
    echo "  Status: SUCCESS"
    echo
    
    # 実行ファイルの確認
    if [ "$GENERATOR" = "Xcode" ]; then
        RUNTIME_PATH="$BUILD_TYPE/Runtime"
        EDITOR_PATH="$BUILD_TYPE/Editor"
    else
        RUNTIME_PATH="bin/Runtime"
        EDITOR_PATH="bin/Editor"
    fi
    
    if [ -f "$RUNTIME_PATH" ]; then
        echo "  Runtime executable: $RUNTIME_PATH"
    fi
    
    if [ -f "$EDITOR_PATH" ]; then
        echo "  Editor executable: $EDITOR_PATH"
    fi
    
    echo
    echo "  To run the application:"
    echo "    cd $BUILD_DIR"
    if [ "$GENERATOR" = "Xcode" ]; then
        echo "    ./$BUILD_TYPE/Runtime"
    else
        echo "    ./bin/Runtime"
    fi
    echo
    echo "========================================="
    
    cd ..
}

# エラーハンドリング
trap 'print_error "Build script interrupted"; exit 1' INT TERM

# メイン処理実行
main "$@"

print_success "Build script completed successfully!"