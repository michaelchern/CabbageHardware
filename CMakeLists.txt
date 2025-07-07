cmake_minimum_required(VERSION 4.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(
    CabbageDisplay
    VERSION 0.5.0
    HOMEPAGE_URL "https://github.com/CoronaEngine/CabbageDisplay"
    LANGUAGES C CXX
)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(CABBAGE_RASTERIZER_BUILD_EXAMPLES "是否构建 CabbageDisplay 示例" ON)
else()
    option(CABBAGE_RASTERIZER_BUILD_EXAMPLES "是否构建 CabbageDisplay 示例" OFF)
endif()

# ==== 子仓库与第三方库 ====
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(Env/Helicon EXCLUDE_FROM_ALL)
add_subdirectory(Src)

if(CABBAGE_RASTERIZER_BUILD_EXAMPLES)
    add_subdirectory(Env/glfw EXCLUDE_FROM_ALL)
    add_subdirectory(Examples)
endif()