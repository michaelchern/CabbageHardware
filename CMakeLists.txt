cmake_minimum_required(VERSION 4.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(
    CabbageDisplayer
    VERSION 0.5.0
    HOMEPAGE_URL "https://github.com/CoronaEngine/CabbageDisplayer"
    LANGUAGES C CXX
)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(CABBAGE_RASTERIZER_BUILD_EXAMPLES "是否构建 CabbageDisplayer 示例" ON)
else()
    option(CABBAGE_RASTERIZER_BUILD_EXAMPLES "是否构建 CabbageDisplayer 示例" OFF)
endif()

add_compile_definitions(
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    NOMINMAX

    $<$<CONFIG:Debug>:CABBAGE_ENGINE_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:CABBAGE_ENGINE_DEBUG>
)

# ==== 子仓库与第三方库 ====
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(Env/Helicon EXCLUDE_FROM_ALL)
add_subdirectory(Src)

if(CABBAGE_RASTERIZER_BUILD_EXAMPLES)
    add_subdirectory(Env/glfw EXCLUDE_FROM_ALL)
    add_subdirectory(Examples)
endif()
