# cmake/EngineUtils.cmake
# ElkGameEngine用のヘルパー関数

# ========================================
# ソースファイル収集関数
# ========================================

# 指定ディレクトリのソースファイルを収集（サブディレクトリ含まず）
function(elk_add_source_directory VAR_NAME DIRECTORY)
    file(GLOB TEMP_SOURCES "${DIRECTORY}/*.cpp" "${DIRECTORY}/*.c")
    file(GLOB TEMP_HEADERS "${DIRECTORY}/*.h" "${DIRECTORY}/*.hpp")
    set(${VAR_NAME} ${TEMP_SOURCES} ${TEMP_HEADERS} PARENT_SCOPE)
    
    list(LENGTH TEMP_SOURCES SOURCE_COUNT)
    list(LENGTH TEMP_HEADERS HEADER_COUNT)
    message(STATUS "  ${DIRECTORY}: ${SOURCE_COUNT} sources, ${HEADER_COUNT} headers")
endfunction()

# 指定ディレクトリのソースファイルを収集（サブディレクトリ含む）
function(elk_add_source_directory_recursive VAR_NAME DIRECTORY)
    file(GLOB_RECURSE TEMP_SOURCES "${DIRECTORY}/*.cpp" "${DIRECTORY}/*.c")
    file(GLOB_RECURSE TEMP_HEADERS "${DIRECTORY}/*.h" "${DIRECTORY}/*.hpp")
    set(${VAR_NAME} ${TEMP_SOURCES} ${TEMP_HEADERS} PARENT_SCOPE)
    
    list(LENGTH TEMP_SOURCES SOURCE_COUNT)
    list(LENGTH TEMP_HEADERS HEADER_COUNT)
    message(STATUS "  ${DIRECTORY} (recursive): ${SOURCE_COUNT} sources, ${HEADER_COUNT} headers")
endfunction()

# プラットフォーム固有ソースを収集
function(elk_add_platform_sources VAR_NAME BASE_DIRECTORY)
    set(PLATFORM_SOURCES "")
    
    if(PLATFORM_WINDOWS)
        file(GLOB WIN_SOURCES "${BASE_DIRECTORY}/Windows/*.cpp" "${BASE_DIRECTORY}/Windows/*.h")
        list(APPEND PLATFORM_SOURCES ${WIN_SOURCES})
        message(STATUS "  Platform sources (Windows): ${WIN_SOURCES}")
    elseif(PLATFORM_LINUX)
        file(GLOB LINUX_SOURCES "${BASE_DIRECTORY}/Linux/*.cpp" "${BASE_DIRECTORY}/Linux/*.h")
        list(APPEND PLATFORM_SOURCES ${LINUX_SOURCES})
        message(STATUS "  Platform sources (Linux): ${LINUX_SOURCES}")
    elseif(PLATFORM_MACOS)
        file(GLOB MACOS_SOURCES "${BASE_DIRECTORY}/macOS/*.cpp" "${BASE_DIRECTORY}/macOS/*.h" "${BASE_DIRECTORY}/macOS/*.mm")
        list(APPEND PLATFORM_SOURCES ${MACOS_SOURCES})
        message(STATUS "  Platform sources (macOS): ${MACOS_SOURCES}")
    endif()
    
    set(${VAR_NAME} ${PLATFORM_SOURCES} PARENT_SCOPE)
endfunction()

# ========================================
# Visual Studio用ソースグループ設定
# ========================================

# ソースファイルをVSのフィルターに整理
function(elk_setup_source_groups TARGET_NAME)
    if(MSVC)
        get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)
        
        foreach(SOURCE_FILE ${TARGET_SOURCES})
            get_filename_component(SOURCE_PATH "${SOURCE_FILE}" PATH)
            string(REPLACE "/" "\\" GROUP_PATH "${SOURCE_PATH}")
            source_group("${GROUP_PATH}" FILES "${SOURCE_FILE}")
        endforeach()
        
        message(STATUS "Setup source groups for ${TARGET_NAME}")
    endif()
endfunction()

# ElkEngine用の標準的なソースグループ設定
function(elk_setup_engine_source_groups)
    if(MSVC)
        # Public headers
        file(GLOB_RECURSE PUBLIC_HEADERS "Public/*.h")
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Public PREFIX "Public" FILES ${PUBLIC_HEADERS})
        
        # Private sources by module
        elk_setup_module_source_group("Core")
        elk_setup_module_source_group("Renderer") 
        elk_setup_module_source_group("Audio")
        elk_setup_module_source_group("Input")
        elk_setup_module_source_group("Math")
        elk_setup_module_source_group("Core/Logger")
        elk_setup_module_source_group("ECS")
        
        # Platform specific
        if(PLATFORM_WINDOWS)
            file(GLOB_RECURSE PLATFORM_SOURCES "Private/Platforms/Windows/*")
            source_group("Private\\Platforms\\Windows" FILES ${PLATFORM_SOURCES})
        elseif(PLATFORM_LINUX)
            file(GLOB_RECURSE PLATFORM_SOURCES "Private/Platforms/Linux/*")
            source_group("Private\\Platforms\\Linux" FILES ${PLATFORM_SOURCES})
        elseif(PLATFORM_MACOS)
            file(GLOB_RECURSE PLATFORM_SOURCES "Private/Platforms/macOS/*")
            source_group("Private\\Platforms\\macOS" FILES ${PLATFORM_SOURCES})
        endif()
    endif()
endfunction()

# モジュール別ソースグループ設定
function(elk_setup_module_source_group MODULE_NAME)
    file(GLOB MODULE_SOURCES "Private/${MODULE_NAME}/*.cpp" "Private/${MODULE_NAME}/*.h")
    if(MODULE_SOURCES)
        source_group("Private\\${MODULE_NAME}" FILES ${MODULE_SOURCES})
    endif()
endfunction()

# ========================================
# ライブラリ設定関数
# ========================================

# ElkEngine用の標準的なライブラリ設定
function(elk_setup_library_properties TARGET_NAME)
    # インクルードディレクトリ設定
    target_include_directories(${TARGET_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Private
    )
    
    # DLL設定
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${TARGET_NAME}
            PRIVATE ENGINE_EXPORTS
            INTERFACE ENGINE_DLL
        )
        
        # Windows DLL設定
        if(WIN32)
            set_target_properties(${TARGET_NAME} PROPERTIES
                WINDOWS_EXPORT_ALL_SYMBOLS FALSE  # __declspec(dllexport)を使用
            )
        endif()
    endif()
    
    # Visual Studio固有設定
    if(MSVC)
        set_target_properties(${TARGET_NAME} PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            FOLDER "ElkEngine"
        )
    endif()
    
    message(STATUS "Setup library properties for ${TARGET_NAME}")
endfunction()

# プラットフォーム固有のライブラリリンク
function(elk_link_platform_libraries TARGET_NAME)
    if(PLATFORM_WINDOWS)
        target_link_libraries(${TARGET_NAME}
            PRIVATE
                d3d11.lib
                dxgi.lib
                d3dcompiler.lib
                winmm.lib
                xinput.lib
                ole32.lib
                oleaut32.lib
                user32.lib
                gdi32.lib
                shell32.lib
                kernel32.lib
        )
        message(STATUS "Linked Windows platform libraries to ${TARGET_NAME}")
        
    elseif(PLATFORM_LINUX)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(X11 REQUIRED x11)
        pkg_check_modules(XRANDR REQUIRED xrandr)
        
        target_link_libraries(${TARGET_NAME}
            PRIVATE
                ${X11_LIBRARIES}
                ${XRANDR_LIBRARIES}
                GL
                pthread
                dl
                rt
        )
        message(STATUS "Linked Linux platform libraries to ${TARGET_NAME}")
        
    elseif(PLATFORM_MACOS)
        find_library(COCOA_FRAMEWORK Cocoa)
        find_library(OPENGL_FRAMEWORK OpenGL)
        find_library(IOKIT_FRAMEWORK IOKit)
        find_library(COREVIDEO_FRAMEWORK CoreVideo)
        
        target_link_libraries(${TARGET_NAME}
            PRIVATE
                ${COCOA_FRAMEWORK}
                ${OPENGL_FRAMEWORK}
                ${IOKIT_FRAMEWORK}
                ${COREVIDEO_FRAMEWORK}
        )
        message(STATUS "Linked macOS platform libraries to ${TARGET_NAME}")
    endif()
endfunction()

# ========================================
# ユーティリティ関数
# ========================================

# ターゲットのソースファイル数を表示
function(elk_print_target_info TARGET_NAME)
    get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)
    list(LENGTH TARGET_SOURCES SOURCE_COUNT)
    
    get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
    
    message(STATUS "Target: ${TARGET_NAME} (${TARGET_TYPE})")
    message(STATUS "  Source files: ${SOURCE_COUNT}")
endfunction()

# プリコンパイル済みヘッダー設定（CMake 3.16+）
function(elk_setup_precompiled_header TARGET_NAME PCH_HEADER)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
        target_precompile_headers(${TARGET_NAME} PRIVATE ${PCH_HEADER})
        message(STATUS "Setup PCH for ${TARGET_NAME}: ${PCH_HEADER}")
    else()
        message(WARNING "CMake 3.16+ required for precompiled headers")
    endif()
endfunction()

# デバッグ用: 変数の内容を表示
function(elk_debug_print_var VAR_NAME)
    message(STATUS "DEBUG: ${VAR_NAME} = ${${VAR_NAME}}")
endfunction()

# コンパイラ警告の設定
function(elk_setup_compiler_warnings TARGET_NAME)
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            /W4          # 警告レベル4
            /w14640      # スレッドセーフでない静的ローカル変数
            /w14826      # 符号付きから符号なしへの変換
            /w14905      # ワイド文字列リテラルから'LPSTR'への変換
            /w14906      # 文字列リテラルから'LPWSTR'への変換
            /w14928      # 不正なコピー初期化
        )
    else()
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wextra
            -Wshadow
            -Wformat=2
            -Wunused
            -Wnull-dereference
            -Wdouble-promotion
        )
        
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${TARGET_NAME} PRIVATE
                -Wmisleading-indentation
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
            )
        endif()
    endif()
    
    message(STATUS "Setup compiler warnings for ${TARGET_NAME}")
endfunction()