# ElkGameEngine/CMakeLists.txt
cmake_minimum_required(VERSION 4.0)
project(ElkGameEngine VERSION 0.1.0 LANGUAGES CXX)

# --- 共通 cmake モジュールをロード ---
set(ELK_CMAKE_DIR "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${ELK_CMAKE_DIR}")

if(EXISTS "${ELK_CMAKE_DIR}/EngineUtils.cmake")
    include("${ELK_CMAKE_DIR}/EngineUtils.cmake")
    message(STATUS "Included EngineUtils.cmake from ${ELK_CMAKE_DIR}")
else()
    message(WARNING "EngineUtils.cmake not found in ${ELK_CMAKE_DIR}; using internal fallbacks.")
    
    # フォールバック関数（最小限）
    function(elk_add_source_directory_recursive outVar dir)
        file(GLOB_RECURSE _elk_tmp_files
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cxx
        )
        set(${outVar} ${_elk_tmp_files} PARENT_SCOPE)
    endfunction()
    
    function(elk_add_source_directory outVar dir)
        file(GLOB _elk_tmp_files
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cxx
        )
        set(${outVar} ${_elk_tmp_files} PARENT_SCOPE)
    endfunction()
    
    function(elk_add_platform_sources outVar dir)
        elk_add_source_directory(${outVar} ${dir})
    endfunction()
    
    # その他のno-op関数
    function(elk_setup_engine_source_groups)
    endfunction()
    function(elk_setup_library_properties target)
    endfunction()
    function(elk_link_platform_libraries target)
    endfunction()
    function(elk_setup_compiler_warnings target)
    endfunction()
    function(elk_print_target_info target)
    endfunction()
endif()

# C++20設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 出力ディレクトリ設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_CONFIGURATION_TYPES)
    foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${cfg} cfg_up)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg_up} ${CMAKE_BINARY_DIR}/bin/${cfg})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${cfg_up} ${CMAKE_BINARY_DIR}/bin/${cfg})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${cfg_up} ${CMAKE_BINARY_DIR}/lib/${cfg})
    endforeach()
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

# ビルド情報表示
message(STATUS "")
message(STATUS "=== ElkGameEngine Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "Build editor: ${BUILD_EDITOR}")
message(STATUS "Build samples: ${BUILD_SAMPLES}")
message(STATUS "========================================")
message(STATUS "")