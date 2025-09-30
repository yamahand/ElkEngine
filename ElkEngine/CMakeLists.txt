# ElkEngine/CMakeLists.txt

message(STATUS "Configuring ElkEngine...")

# C++23設定
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- 追加: 共通 cmake モジュールをロード ---
set(ELK_CMAKE_DIR "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${ELK_CMAKE_DIR}")
if(EXISTS "${ELK_CMAKE_DIR}/EngineUtils.cmake")
    include("${ELK_CMAKE_DIR}/EngineUtils.cmake")
    message(STATUS "Included EngineUtils.cmake from ${ELK_CMAKE_DIR}")
else()
    message(WARNING "EngineUtils.cmake not found in ${ELK_CMAKE_DIR}; continuing without it (fallbacks may be used).")
endif()
# --- ここまで追加 ---

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
elk_add_source_directory(ENGINE_LOGGER_SOURCES "Private/Core/Logger")
elk_add_source_directory(ENGINE_ECS_SOURCES "Private/ECS")

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
    ${ENGINE_ECS_SOURCES}
    ${ENGINE_LOGGER_SOURCES}
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
        ELK_ENGINE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
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

# 外部ライブラリリンク: ThirdParty 側で ThirdParty::spdlog が作られていればリンクする
message(STATUS "Checking for ThirdParty::spdlog target...")
if(TARGET spdlog::spdlog)
    target_link_libraries(ElkEngine PUBLIC spdlog::spdlog)
    target_compile_definitions(ElkEngine PUBLIC ELK_USE_SPDLOG=1)
    message(STATUS "Linked spdlog::spdlog to ElkEngine")
elseif(TARGET spdlog)
    target_link_libraries(ElkEngine PUBLIC spdlog)
    target_compile_definitions(ElkEngine PUBLIC ELK_USE_SPDLOG=1)
    message(STATUS "Linked spdlog to ElkEngine")
else()
    message(STATUS "spdlog not available for linking")
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
    
    # MSVC のプリプロセッサ拡張を有効化 (/Zc:preprocessor)
    # これは __VA_OPT__ を使うソースで必要となる
    # ジェネレータ式でコンパイラ種別と構成をガードしてターゲットに付与する
    target_compile_options(ElkEngine PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
    )
endif()

# ========================================
# デバッグ情報表示
# ========================================

elk_print_target_info(ElkEngine)

message(STATUS "ElkEngine configuration completed")
message(STATUS "")