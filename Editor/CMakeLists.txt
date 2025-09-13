# Editor/CMakeLists.txt

message(STATUS "Configuring Editor...")

# ========================================
# ソースファイル収集
# ========================================

elk_add_source_directory_recursive(EDITOR_SOURCES "Source")

# リソースファイル（Windows）
if(WIN32)
    file(GLOB EDITOR_RESOURCES "Resources/*.rc" "Resources/*.ico")
    if(EDITOR_RESOURCES)
        list(APPEND EDITOR_SOURCES ${EDITOR_RESOURCES})
        message(STATUS "  Found Windows resources: ${EDITOR_RESOURCES}")
    endif()
endif()

# ========================================
# プラットフォーム別設定
# ========================================

set(EDITOR_TYPE "")
if(WIN32)
    # Windows GUI アプリケーション
    set(EDITOR_TYPE WIN32)
endif()

# ========================================
# 実行ファイル作成
# ========================================

add_executable(Editor ${EDITOR_TYPE} ${EDITOR_SOURCES})

# ========================================
# ライブラリリンク
# ========================================

# ElkEngine をリンク
target_link_libraries(Editor
    PRIVATE
        ElkEngine
)

# プラットフォーム固有ライブラリ
if(WIN32)
    target_link_libraries(Editor
        PRIVATE
            comctl32.lib     # Windows Common Controls
            shell32.lib      # Shell API
            ole32.lib        # OLE
            oleaut32.lib     # OLE Automation
            uuid.lib         # UUID
            advapi32.lib     # Registry API
            ws2_32.lib       # Winsock (エディタ通信用)
    )
elseif(PLATFORM_LINUX)
    # GTK3 または Qt5 ライブラリ（将来的に）
    target_link_libraries(Editor
        PRIVATE
            pthread
            dl
    )
endif()

# ========================================
# インクルードディレクトリ
# ========================================

target_include_directories(Editor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# ========================================
# プリプロセッサ定義
# ========================================

target_compile_definitions(Editor
    PRIVATE
        ELK_EDITOR
        $<$<CONFIG:Debug>:ELK_DEBUG>
        $<$<CONFIG:Release>:ELK_RELEASE>
        ELK_EDITOR_VERSION="${PROJECT_VERSION}"
)

# ========================================
# コンパイラ設定
# ========================================

elk_setup_compiler_warnings(Editor)

# ========================================
# Visual Studio設定
# ========================================

if(MSVC)
    # ソースグループ設定
    elk_setup_source_groups(Editor)
    
    # エディタ用の作業ディレクトリ設定
    set_target_properties(Editor PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        FOLDER "Applications"
    )
    
    # マニフェストファイル（DPI対応など）
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Editor.manifest")
        set_target_properties(Editor PROPERTIES
            VS_DPI_AWARE "PerMonitor"
        )
    endif()
endif()

# ========================================
# 実行時DLL設定（Windows）
# ========================================

if(WIN32 AND BUILD_SHARED_LIBS)
    # ElkEngine.dll を実行ファイルと同じディレクトリにコピー
    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ElkEngine>
            $<TARGET_FILE_DIR:Editor>
        COMMENT "Copying ElkEngine.dll to Editor directory"
    )
endif()

# ========================================
# インストール設定
# ========================================

install(TARGETS Editor
    DESTINATION bin
)

# エディタ設定ファイル
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Config")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Config
        DESTINATION bin
    )
endif()

# ========================================
# デバッグ情報
# ========================================

elk_print_target_info(Editor)

message(STATUS "Editor configuration completed")
message(STATUS "")