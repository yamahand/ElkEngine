# ElkGameEngine/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(ElkGameEngine VERSION 1.0.0 LANGUAGES CXX)

# グローバル設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ビルド構成設定
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

# 出力ディレクトリ設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Multi-config generators (Visual Studio) 対応
foreach(OUTPUTCONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG}/lib)
endforeach()

# オプション
option(BUILD_SHARED_LIBS "Build shared libraries (DLL)" OFF)
option(BUILD_EDITOR "Build editor application" ON)
option(BUILD_TOOLS "Build development tools" OFF)
option(BUILD_SAMPLES "Build sample projects" OFF)
option(ELK_ENABLE_LOGGING "Enable logging system" ON)
option(ELK_ENABLE_PROFILER "Enable profiler" $<$<CONFIG:Debug>:ON>)

# プラットフォーム検出
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    set(PLATFORM_NAME "Windows")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
    set(PLATFORM_NAME "Linux")
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    set(PLATFORM_NAME "macOS")
endif()

message(STATUS "Building for platform: ${PLATFORM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Shared libraries: ${BUILD_SHARED_LIBS}")

# コンパイラ設定
if(MSVC)
    # Visual Studio 設定
    add_compile_options(
        /W4                    # 警告レベル4
        /WX-                   # 警告をエラーにしない（調整可能）
        /MP                    # マルチプロセッサコンパイル
        /permissive-           # 標準準拠モード
    )
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX               # min/maxマクロを無効化
        WIN32_LEAN_AND_MEAN    # Windows.hの軽量化
    )
    
    # 静的ランタイムリンク（Release時）
    if(NOT BUILD_SHARED_LIBS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
    
    # デバッグ情報生成
    add_compile_options($<$<CONFIG:RelWithDebInfo>:/Zi>)
    add_link_options($<$<CONFIG:RelWithDebInfo>:/DEBUG>)
    
else()
    # GCC/Clang 設定
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter  # 未使用パラメータ警告を抑制
    )
    
    # デバッグ設定
    add_compile_options($<$<CONFIG:Debug>:-g>)
    add_compile_options($<$<CONFIG:Debug>:-O0>)
    
    # リリース設定
    add_compile_options($<$<CONFIG:Release>:-O3>)
    add_compile_options($<$<CONFIG:Release>:-DNDEBUG>)
endif()

# グローバルプリプロセッサ定義
add_compile_definitions(
    ELK_ENGINE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    ELK_ENGINE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    ELK_PLATFORM_${PLATFORM_NAME}
    $<$<CONFIG:Debug>:ELK_DEBUG>
    $<$<CONFIG:Release>:ELK_RELEASE>
    $<$<BOOL:${ELK_ENABLE_LOGGING}>:ELK_ENABLE_LOGGING>
    $<$<BOOL:${ELK_ENABLE_PROFILER}>:ELK_ENABLE_PROFILER>
)

# ヘルパー関数読み込み
include(cmake/EngineUtils.cmake)

# サブディレクトリ
add_subdirectory(ThirdParty)
add_subdirectory(ElkEngine)
add_subdirectory(Runtime)

if(BUILD_EDITOR)
    add_subdirectory(Editor)
endif()

if(BUILD_TOOLS)
    add_subdirectory(Tools)
endif()

if(BUILD_SAMPLES)
    add_subdirectory(Samples)
endif()

# インストール設定（配布用）
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install path" FORCE)
endif()

# CPack設定（パッケージ化）
set(CPACK_PACKAGE_NAME "ElkGameEngine")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Elk Game Engine")
set(CPACK_PACKAGE_VENDOR "ElkEngine Team")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)

# ビルド情報表示
message(STATUS "")
message(STATUS "=== ElkGameEngine Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Platform: ${PLATFORM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "Build editor: ${BUILD_EDITOR}")
message(STATUS "Build tools: ${BUILD_TOOLS}")
message(STATUS "Build samples: ${BUILD_SAMPLES}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "========================================")
message(STATUS "")