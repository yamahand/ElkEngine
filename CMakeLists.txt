# ElkGameEngine/CMakeLists.txt
cmake_minimum_required(VERSION 4.0)
project(ElkGameEngine LANGUAGES CXX)

# C++20設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# モジュールサポート（必要なら残す）
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP ON)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")

# 出力をビルドツリー下の bin/<Config> / lib/<Config> に固定（マルチコンフィグ対応）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_CONFIGURATION_TYPES) # multi-config generators (Visual Studio 等)
    foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${cfg} cfg_up)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg_up} ${CMAKE_BINARY_DIR}/bin/${cfg})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${cfg_up} ${CMAKE_BINARY_DIR}/bin/${cfg})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${cfg_up} ${CMAKE_BINARY_DIR}/lib/${cfg})
    endforeach()
else()
    # single-config generators (Ninja, Makefile) 用に BUILD_TYPE を参照する場合は環境変数で指定
    if(DEFINED ENV{BUILD_TYPE})
        set(CMAKE_BUILD_TYPE $ENV{BUILD_TYPE} CACHE STRING "Build type")
    endif()
endif()

# プロジェクト構成
add_subdirectory(ElkEngine)
add_subdirectory(EditorBridge)
add_subdirectory(Runtime)

if(BUILD_EDITOR)
    message(STATUS "C# Editor: Editor/ElkEditor.csproj")
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