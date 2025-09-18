# ElkGameEngine/CMakeLists.txt
cmake_minimum_required(VERSION 4.0)
project(ElkGameEngine LANGUAGES CXX)

# --- 追加: 共通 cmake モジュールをロード（存在すれば include、なければフォールバック実装を追加） ---
set(ELK_CMAKE_DIR "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${ELK_CMAKE_DIR}")

if(EXISTS "${ELK_CMAKE_DIR}/EngineUtils.cmake")
    include("${ELK_CMAKE_DIR}/EngineUtils.cmake")
    message(STATUS "Included EngineUtils.cmake from ${ELK_CMAKE_DIR}")
else()
    message(WARNING "EngineUtils.cmake not found in ${ELK_CMAKE_DIR}; using internal fallbacks.")
endif()

# フォールバック: elk_* ヘルパーが未定義なら簡易実装を提供
if(NOT COMMAND elk_add_source_directory_recursive)
    function(elk_add_source_directory_recursive outVar dir)
        file(GLOB_RECURSE _elk_tmp_files
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.hh
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cxx
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cc
        )
        set(${outVar} ${_elk_tmp_files} PARENT_SCOPE)
    endfunction()
    message(STATUS "Registered fallback: elk_add_source_directory_recursive")
endif()

if(NOT COMMAND elk_add_source_directory)
    function(elk_add_source_directory outVar dir)
        file(GLOB _elk_tmp_files
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.hh
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cxx
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cc
        )
        set(${outVar} ${_elk_tmp_files} PARENT_SCOPE)
    endfunction()
    message(STATUS "Registered fallback: elk_add_source_directory")
endif()

if(NOT COMMAND elk_add_platform_sources)
    function(elk_add_platform_sources outVar dir)
        elk_add_source_directory(${outVar} ${dir})
    endfunction()
    message(STATUS "Registered fallback: elk_add_platform_sources")
endif()

if(NOT COMMAND elk_setup_engine_source_groups)
    function(elk_setup_engine_source_groups) # no-op fallback
    endfunction()
endif()
if(NOT COMMAND elk_setup_library_properties)
    function(elk_setup_library_properties target) # no-op fallback
    endfunction()
endif()
if(NOT COMMAND elk_link_platform_libraries)
    function(elk_link_platform_libraries target) # no-op fallback
    endfunction()
endif()
if(NOT COMMAND elk_setup_compiler_warnings)
    function(elk_setup_compiler_warnings target) # no-op fallback
    endfunction()
endif()
if(NOT COMMAND elk_print_target_info)
    function(elk_print_target_info target) # no-op fallback
    endfunction()
endif()
# --- ここまで追加 ---

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
add_subdirectory(ThirdParty)
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