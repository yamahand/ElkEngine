cmake_minimum_required(VERSION 3.20)
project(EditorBridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Configuring EditorBridge...")

# ソース収集（プロジェクトの elk_* ヘルパーがあれば使用、なければフォールバック）
if(COMMAND elk_add_source_directory_recursive)
    elk_add_source_directory_recursive(BRIDGE_PUBLIC_HEADERS "Public")
else()
    file(GLOB_RECURSE BRIDGE_PUBLIC_HEADERS
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Public/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Public/*.hpp
    )
endif()

if(COMMAND elk_add_source_directory)
    elk_add_source_directory(BRIDGE_PRIVATE_SOURCES "Private")
else()
    file(GLOB_RECURSE BRIDGE_PRIVATE_SOURCES
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Private/*.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/Private/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Private/*.h
    )
endif()

set(BRIDGE_SOURCES
    ${BRIDGE_PUBLIC_HEADERS}
    ${BRIDGE_PRIVATE_SOURCES}
)

# ライブラリ作成
add_library(EditorBridge SHARED ${BRIDGE_SOURCES})

# ヘッダ検索パス
target_include_directories(EditorBridge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/ElkEngine/Public>  # エンジンの公開ヘッダ参照
)

# エンジン本体にリンク（ElkEngine ターゲットが存在することを前提）
if(TARGET ElkEngine)
    target_link_libraries(EditorBridge PRIVATE ElkEngine)
endif()

# エクスポート定義（ヘッダで BRIDGE_API 等を切り替えるため）
target_compile_definitions(EditorBridge PRIVATE BRIDGE_EXPORTS)

# ターゲットプロパティ
set_target_properties(EditorBridge PROPERTIES
    FOLDER "EditorBridge"
    POSITION_INDEPENDENT_CODE ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# マルチコンフィグジェネレータでも出力を安定化
if(CMAKE_CONFIGURATION_TYPES)
    foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${cfg} cfg_up)
        set_target_properties(EditorBridge PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${cfg_up}" "${CMAKE_BINARY_DIR}/bin/${cfg}"
            "LIBRARY_OUTPUT_DIRECTORY_${cfg_up}" "${CMAKE_BINARY_DIR}/bin/${cfg}"
            "ARCHIVE_OUTPUT_DIRECTORY_${cfg_up}" "${CMAKE_BINARY_DIR}/lib/${cfg}"
        )
    endforeach()
endif()

# Visual Studio 用のフォルダ分け（任意）
set_target_properties(EditorBridge PROPERTIES VS_FOLDER "EditorBridge")