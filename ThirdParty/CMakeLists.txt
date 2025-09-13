# ThirdParty/CMakeLists.txt

message(STATUS "Configuring ThirdParty libraries...")

# ========================================
# 外部ライブラリ管理設定
# ========================================

# FetchContent を使用した自動取得
include(FetchContent)

# オプション設定
option(ELK_FETCHCONTENT_AUTO "Use FetchContent for automatic dependency management" ON)
option(ELK_USE_GLFW "Use GLFW for windowing" OFF)
option(ELK_USE_GLM "Use GLM for mathematics" OFF)
option(ELK_USE_STB "Use STB for image loading" OFF)
option(ELK_USE_IMGUI "Use Dear ImGui for debugging UI" ON)

# ========================================
# GLFW（ウィンドウ管理）
# ========================================

if(ELK_USE_GLFW AND ELK_FETCHCONTENT_AUTO)
    message(STATUS "Fetching GLFW...")
    
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
        GIT_SHALLOW TRUE
    )
    
    # GLFW のビルドオプション設定
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(glfw)
    
    # エイリアス作成
    add_library(ThirdParty::GLFW ALIAS glfw)
    
    # フォルダ設定（Visual Studio用）
    if(TARGET glfw)
        set_target_properties(glfw PROPERTIES FOLDER "ThirdParty/GLFW")
    endif()
    
    message(STATUS "  GLFW configured successfully")
endif()

# ========================================
# GLM（数学ライブラリ）
# ========================================

if(ELK_USE_GLM AND ELK_FETCHCONTENT_AUTO)
    message(STATUS "Fetching GLM...")
    
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(glm)
    
    # エイリアス作成
    add_library(ThirdParty::GLM ALIAS glm::glm)
    
    message(STATUS "  GLM configured successfully")
endif()

# ========================================
# STB（画像読み込み）
# ========================================

if(ELK_USE_STB AND ELK_FETCHCONTENT_AUTO)
    message(STATUS "Fetching STB...")
    
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(stb)
    
    # STB用のインターフェースライブラリ作成
    add_library(stb_interface INTERFACE)
    target_include_directories(stb_interface INTERFACE ${stb_SOURCE_DIR})
    
    # エイリアス作成
    add_library(ThirdParty::STB ALIAS stb_interface)
    
    message(STATUS "  STB configured successfully")
endif()

# ========================================
# Dear ImGui（デバッグ用UI）
# ========================================

if(ELK_USE_IMGUI AND ELK_FETCHCONTENT_AUTO)
    message(STATUS "Fetching Dear ImGui...")
    
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.89.9
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(imgui)
    
    # ImGui用のライブラリ作成
    add_library(imgui_lib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    )
    
    target_include_directories(imgui_lib
        PUBLIC
            ${imgui_SOURCE_DIR}
    )
    
    # プラットフォーム固有のバックエンド
    if(ELK_USE_GLFW AND TARGET glfw)
        target_sources(imgui_lib PRIVATE
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )
        
        target_include_directories(imgui_lib PRIVATE
            ${imgui_SOURCE_DIR}/backends
        )
        
        target_link_libraries(imgui_lib PRIVATE glfw)
    endif()
    
    # Windows DirectX11 バックエンド
    if(WIN32)
        target_sources(imgui_lib PRIVATE
            ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
        )
    endif()
    
    # エイリアス作成
    add_library(ThirdParty::ImGui ALIAS imgui_lib)
    
    # フォルダ設定
    set_target_properties(imgui_lib PROPERTIES FOLDER "ThirdParty/ImGui")
    
    message(STATUS "  Dear ImGui configured successfully")
endif()

# ========================================
# 手動配置ライブラリ
# ========================================

# FMOD（商用ライブラリの例）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/FMOD")
    message(STATUS "Found FMOD in ThirdParty directory")
    
    add_library(fmod_interface INTERFACE)
    
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(FMOD_ARCH "x64")
        else()
            set(FMOD_ARCH "x86")
        endif()
        
        target_include_directories(fmod_interface INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/FMOD/inc
        )
        
        target_link_directories(fmod_interface INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/FMOD/lib/${FMOD_ARCH}
        )
        
        target_link_libraries(fmod_interface INTERFACE
            fmod_vc.lib
            fmodstudio_vc.lib
        )
    endif()
    
    add_library(ThirdParty::FMOD ALIAS fmod_interface)
    message(STATUS "  FMOD configured successfully")
endif()

# DirectXTK（Windows）
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/DirectXTK")
    message(STATUS "Found DirectXTK in ThirdParty directory")
    
    add_subdirectory(DirectXTK)
    
    if(TARGET DirectXTK)
        add_library(ThirdParty::DirectXTK ALIAS DirectXTK)
        set_target_properties(DirectXTK PROPERTIES FOLDER "ThirdParty/DirectXTK")
        message(STATUS "  DirectXTK configured successfully")
    endif()
endif()

# ========================================
# システムライブラリ検索
# ========================================

# OpenGL
find_package(OpenGL)
if(OpenGL_FOUND)
    add_library(ThirdParty::OpenGL ALIAS OpenGL::GL)
    message(STATUS "Found system OpenGL: ${OPENGL_LIBRARIES}")
endif()

# Vulkan
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    add_library(ThirdParty::Vulkan ALIAS Vulkan::Vulkan)
    message(STATUS "Found system Vulkan: ${Vulkan_LIBRARIES}")
endif()

# ========================================
# サマリー表示
# ========================================

message(STATUS "")
message(STATUS "=== ThirdParty Libraries Summary ===")
message(STATUS "GLFW:     ${ELK_USE_GLFW}")
message(STATUS "GLM:      ${ELK_USE_GLM}")
message(STATUS "STB:      ${ELK_USE_STB}")
message(STATUS "ImGui:    ${ELK_USE_IMGUI}")

if(TARGET ThirdParty::FMOD)
    message(STATUS "FMOD:     Found (manual)")
endif()

if(TARGET ThirdParty::DirectXTK)
    message(STATUS "DirectXTK: Found (manual)")
endif()

if(TARGET ThirdParty::OpenGL)
    message(STATUS "OpenGL:   Found (system)")
endif()

if(TARGET ThirdParty::Vulkan)
    message(STATUS "Vulkan:   Found (system)")
endif()

message(STATUS "====================================")
message(STATUS "")

message(STATUS "ThirdParty configuration completed")