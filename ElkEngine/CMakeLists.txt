# ElkEngine/CMakeLists.txt

message(STATUS "Configuring ElkEngine...")

# ========================================
# ソースファイル収集
# ========================================

# Public ヘッダー（全て含める）
elk_add_source_directory_recursive(ENGINE_PUBLIC_HEADERS "Public")

# Private モジュール別ソース収集
elk_add_source_directory(ENGINE_CORE_SOURCES "Private/Core")
elk_add_source_directory(ENGINE_RENDERER_SOURCES "Private/Renderer")
elk_add_source_directory(ENGINE_AUDIO_SOURCES "Private/Audio")
elk_add_source_directory(ENGINE_INPUT_SOURCES "Private/Input")
elk_add_source_directory(ENGINE_MATH_SOURCES "Private/Math")

# プラットフォーム固有ソース
elk_add_platform_sources(ENGINE_PLATFORM_SOURCES "Private/Platforms")

# 全ソースファイルをまとめる
set(ENGINE_ALL_SOURCES
    ${ENGINE_PUBLIC_HEADERS}
    ${ENGINE_CORE_SOURCES}
    ${ENGINE_RENDERER_SOURCES}
    ${ENGINE_AUDIO_SOURCES}
    ${ENGINE_INPUT_SOURCES}
    ${ENGINE_MATH_SOURCES}
    ${ENGINE_PLATFORM_SOURCES}
)

# ========================================
# ライブラリ作成
# ========================================

if(BUILD_SHARED_LIBS)
    add_library(ElkEngine SHARED ${ENGINE_ALL_SOURCES})
    message(STATUS "Building ElkEngine as shared library (DLL)")
else()
    add_library(ElkEngine STATIC ${ENGINE_ALL_SOURCES})
    message(STATUS "Building ElkEngine as static library")
endif()

# ========================================
# ライブラリ設定
# ========================================

# 標準的なライブラリプロパティ設定
elk_setup_library_properties(ElkEngine)

# プラットフォーム固有ライブラリリンク
elk_link_platform_libraries(ElkEngine)

# コンパイラ警告設定
elk_setup_compiler_warnings(ElkEngine)

# ========================================
# プリプロセッサ定義
# ========================================

target_compile_definitions(ElkEngine
    PUBLIC
        ELK_ENGINE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        ELK_ENGINE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        ELK_PLATFORM_${PLATFORM_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:ELK_DEBUG>
        $<$<CONFIG:Release>:ELK_RELEASE>
        $<$<BOOL:${ELK_ENABLE_LOGGING}>:ELK_ENABLE_LOGGING>
        $<$<BOOL:${ELK_ENABLE_PROFILER}>:ELK_ENABLE_PROFILER>
)

# ========================================
# 外部ライブラリリンク
# ========================================

# ThirdPartyライブラリのリンク（存在する場合）
if(TARGET ThirdParty::GLFW)
    target_link_libraries(ElkEngine PRIVATE ThirdParty::GLFW)
    message(STATUS "Linked GLFW to ElkEngine")
endif()

if(TARGET ThirdParty::GLM)
    target_link_libraries(ElkEngine PRIVATE ThirdParty::GLM)
    message(STATUS "Linked GLM to ElkEngine")
endif()

if(TARGET ThirdParty::STB)
    target_link_libraries(ElkEngine PRIVATE ThirdParty::STB)
    message(STATUS "Linked STB to ElkEngine")
endif()

# ========================================
# Visual Studio設定
# ========================================

if(MSVC)
    # ソースグループ設定
    elk_setup_engine_source_groups()
    
    # スタートアッププロジェクト設定
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Runtime)
    
    # フォルダ設定
    set_target_properties(ElkEngine PROPERTIES FOLDER "Engine")
    
    # プリコンパイル済みヘッダー（オプション）
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Private/Core/EnginePCH.h")
        elk_setup_precompiled_header(ElkEngine "Private/Core/EnginePCH.h")
    endif()
endif()

# ========================================
# インストール設定
# ========================================

# ヘッダーファイルインストール
install(DIRECTORY Public/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# ライブラリインストール
install(TARGETS ElkEngine
    EXPORT ElkEngineTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# CMakeバージョンチェック後にエクスポート設定
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    # CMake設定ファイルインストール
    install(EXPORT ElkEngineTargets
        FILE ElkEngineTargets.cmake
        NAMESPACE elk::
        DESTINATION lib/cmake/ElkEngine
    )
    
    # 簡易Config.cmakeファイル生成
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ElkEngineConfig.cmake"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/ElkEngineTargets.cmake\")
set(ElkEngine_FOUND TRUE)
")
    
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ElkEngineConfig.cmake"
        DESTINATION lib/cmake/ElkEngine
    )
    
    message(STATUS "Package export configured for CMAKE ${CMAKE_VERSION}")
else()
    message(WARNING "CMAKE ${CMAKE_VERSION} detected. Package export disabled (requires 3.15+)")
endif()

# ========================================
# デバッグ情報表示
# ========================================

elk_print_target_info(ElkEngine)

message(STATUS "ElkEngine configuration completed")
message(STATUS "")