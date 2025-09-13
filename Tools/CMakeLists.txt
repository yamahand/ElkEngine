# Tools/CMakeLists.txt

message(STATUS "Configuring Tools...")

# Asset Processor
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/AssetProcessor")
    add_subdirectory(AssetProcessor)
endif()

# Shader Compiler
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ShaderCompiler")
    add_subdirectory(ShaderCompiler)
endif()

message(STATUS "Tools configuration completed")
