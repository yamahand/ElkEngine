# Runtime/CMakeLists.txt

message(STATUS "Configuring Runtime...")

# ========================================
# ソースファイル収集
# ========================================

elk_add_source_directory_recursive(RUNTIME_SOURCES "Source")

# Visual Studio のフィルターを Runtime ディレクトリ基準の相対パスにする
if(MSVC)
    foreach(_src IN LISTS RUNTIME_SOURCES)
        file(RELATIVE_PATH _rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_src}")
        get_filename_component(_dir "${_rel}" PATH)
        if(_dir STREQUAL "")
            set(_dir "Source")
        endif()
        string(REPLACE "/" "\\" _group "${_dir}")
        source_group("${_group}" FILES "${_src}")
    endforeach()
endif()

# ========================================
# 実行ファイル作成
# ========================================

add_executable(Runtime ${RUNTIME_SOURCES})

# ========================================
# ライブラリリンク
# ========================================

# ElkEngine をリンク
target_link_libraries(Runtime
    PRIVATE
        ElkEngine
)

# ========================================
# インクルードディレクトリ
# ========================================

target_include_directories(Runtime
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# ========================================
# プリプロセッサ定義
# ========================================

target_compile_definitions(Runtime
    PRIVATE
        $<$<CONFIG:Debug>:ELK_DEBUG>
        $<$<CONFIG:Release>:ELK_RELEASE>
        ELK_RUNTIME_VERSION="${PROJECT_VERSION}"
)

# ========================================
# コンパイラ設定
# ========================================

elk_setup_compiler_warnings(Runtime)

# ========================================
# Visual Studio設定
# ========================================

if(MSVC)
    # ソースグループ設定
    elk_setup_source_groups(Runtime)
    
    # 作業ディレクトリ設定
    set_target_properties(Runtime PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Assets
        FOLDER "Applications"
    )
    
    # スタートアッププロジェクト設定
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Runtime)
endif()

# ========================================
# アセットのコピー（開発用）
# ========================================

# Assets フォルダが存在する場合、ビルド出力ディレクトリにコピー
if(EXISTS "${CMAKE_SOURCE_DIR}/Assets")
    # カスタムターゲットでアセットをコピー
    add_custom_target(CopyAssets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Assets"
            "$<TARGET_FILE_DIR:Runtime>/Assets"
        COMMENT "Copying assets to build directory"
        VERBATIM
    )
    
    # Runtime の依存関係に追加
    add_dependencies(Runtime CopyAssets)
    
    message(STATUS "Assets will be copied to build directory")
endif()

# ========================================
# 実行時DLL設定（Windows）
# ========================================

if(WIN32 AND BUILD_SHARED_LIBS)
    # ElkEngine.dll を実行ファイルと同じディレクトリにコピー
    add_custom_command(TARGET Runtime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ElkEngine>
            $<TARGET_FILE_DIR:Runtime>
        COMMENT "Copying ElkEngine.dll to Runtime directory"
    )
endif()

# ========================================
# インストール設定
# ========================================

install(TARGETS Runtime
    DESTINATION bin
)

# アセットもインストール
if(EXISTS "${CMAKE_SOURCE_DIR}/Assets")
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/Assets
        DESTINATION .
    )
endif()

# ========================================
# デバッグ情報
# ========================================

elk_print_target_info(Runtime)

message(STATUS "Runtime configuration completed")
message(STATUS "")