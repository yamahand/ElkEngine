# ThirdParty/CMakeLists.txt

message(STATUS "Configuring ThirdParty libraries...")

# ========================================
# 外部ライブラリ管理設定
# ========================================

include(FetchContent)

# オプション設定
option(ELK_USE_GLFW "Use GLFW for windowing" OFF)
option(ELK_USE_GLM "Use GLM for mathematics" OFF)
option(ELK_USE_STB "Use STB for image loading" OFF)
option(ELK_USE_IMGUI "Use Dear ImGui for debugging UI" ON)
option(ELK_USE_SPDLOG "Use spdlog for logging" ON)

# Helper: フォルダ設定（シンプル版）
function(elk_set_folder_for_target tgt folder)
    if(TARGET ${tgt})
        get_target_property(_imported ${tgt} IMPORTED)
        if(NOT _imported)
            set_target_properties(${tgt} PROPERTIES FOLDER ${folder})
        endif()
    endif()
endfunction()

# ========================================
# GLFW（ウィンドウ管理）
# ========================================

if(ELK_USE_GLFW)
    message(STATUS "Fetching GLFW...")
    
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
        GIT_SHALLOW TRUE
    )
    
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(glfw)
    
    add_library(ThirdParty::GLFW ALIAS glfw)
    elk_set_folder_for_target(glfw "ThirdParty/GLFW")
    
    message(STATUS "  GLFW configured successfully")
endif()

# ========================================
# GLM（数学ライブラリ）
# ========================================

if(ELK_USE_GLM)
    message(STATUS "Fetching GLM...")
    
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(glm)
    add_library(ThirdParty::GLM ALIAS glm::glm)
    
    message(STATUS "  GLM configured successfully")
endif()

# ========================================
# STB（画像読み込み）
# ========================================

if(ELK_USE_STB)
    message(STATUS "Fetching STB...")
    
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(stb)
    
    add_library(stb_interface INTERFACE)
    target_include_directories(stb_interface INTERFACE ${stb_SOURCE_DIR})
    add_library(ThirdParty::STB ALIAS stb_interface)
    
    message(STATUS "  STB configured successfully")
endif()

# ========================================
# Dear ImGui（デバッグ用UI）
# ========================================

if(ELK_USE_IMGUI)
    message(STATUS "Fetching Dear ImGui...")
    
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.3
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(imgui)
    
    add_library(imgui_lib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    )
    
    target_include_directories(imgui_lib PUBLIC ${imgui_SOURCE_DIR})
    add_library(ThirdParty::ImGui ALIAS imgui_lib)
    elk_set_folder_for_target(imgui_lib "ThirdParty/ImGui")
    
    message(STATUS "  Dear ImGui configured successfully")
endif()

# ========================================
# spdlog (logging)
# ========================================

if(ELK_USE_SPDLOG)
    find_package(spdlog CONFIG QUIET)

    if(TARGET spdlog::spdlog)
        message(STATUS "spdlog found as package")
        elk_set_folder_for_target(spdlog "ThirdParty/spdlog")
        target_compile_definitions(spdlog::spdlog PUBLIC SPDLOG_USE_STD_FORMAT)
    else()
        message(STATUS "spdlog not found as package; fetching via FetchContent...")
        FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.15.3
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(spdlog)
        message(STATUS "spdlog fetched successfully")
        elk_set_folder_for_target(spdlog "ThirdParty/spdlog")
        target_compile_definitions(spdlog PUBLIC SPDLOG_USE_STD_FORMAT)
    endif()
endif()

# ========================================
# システムライブラリ検索
# ========================================

# OpenGL
find_package(OpenGL QUIET)
if(OpenGL_FOUND)
    add_library(ThirdParty::OpenGL ALIAS OpenGL::GL)
    message(STATUS "Found system OpenGL")
endif()

# Vulkan
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    add_library(ThirdParty::Vulkan ALIAS Vulkan::Vulkan)
    message(STATUS "Found system Vulkan")
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
message(STATUS "spdlog:   ${ELK_USE_SPDLOG}")

if(OpenGL_FOUND)
    message(STATUS "OpenGL:   Found (system)")
endif()

if(Vulkan_FOUND)
    message(STATUS "Vulkan:   Found (system)")
endif()

message(STATUS "====================================")
message(STATUS "")

message(STATUS "ThirdParty configuration completed")