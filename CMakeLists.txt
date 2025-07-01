# 设置 CMake 最低版本要求(动态匹配当前 CMake 主次版本)
cmake_minimum_required(VERSION 4.0)
set(CMAKE_POLICY_VERSION_MINIMUM "4.0")

# -------------------------------------------------------------------------------
# 项目基本信息配置
# -------------------------------------------------------------------------------
# 定义项目名称 CabbageFramework
# 版本号 0.5.0
# 项目主页 URL
# 指定支持 C 和 C++ 语言
project(
    CabbageFramework
    VERSION 0.5.0
    HOMEPAGE_URL "https://github.com/Cabbage-Engine/CabbageEngine"
    LANGUAGES C CXX
)

if(NOT USED_PRESET_CONFIG)
    message(FATAL_ERROR "请使用CMake预设配置！！！")
endif()

# -------------------------------------------------------------------------------
# CMake策略配置
# -------------------------------------------------------------------------------
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW) # CMP0077: 正确处理 option() 命令的目录作用域
endif()

# -------------------------------------------------------------------------------
# 依赖查找
# -------------------------------------------------------------------------------
# 查找 Python 3.13 版本
# 要求包含解释器(Interpreter)和开发组件(Development)
find_package(Python3 3.13 REQUIRED COMPONENTS Interpreter Development)

# 查找 CUDA 工具链
find_package(CUDAToolkit)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/scripts/pip_install.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/scripts"
)

# -------------------------------------------------------------------------------
# 源代码管理
# -------------------------------------------------------------------------------
# 文件分组函数
function(set_source_groups SOURCE_FILES SOURCE_DIR SOURCE_DIR_NAME)
    foreach(each_source_file IN LISTS SOURCE_FILES)
        get_filename_component(each_source_path "${each_source_file}" PATH)
        file(RELATIVE_PATH each_source_path_rel ${SOURCE_DIR} ${each_source_path})

        # 检查字符串是否为空 空的不分组
        if("${each_source_path_rel}" STREQUAL "")
            source_group("${SOURCE_DIR_NAME}" FILES ${each_source_file})
            continue()
        endif()

        string(REPLACE "/" "\\" each_source_group_path ${each_source_path_rel})
        source_group("${SOURCE_DIR_NAME}/${each_source_group_path}"
            FILES ${each_source_file})
    endforeach()
endfunction()


# 设置源代码目录
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SourceCode)

# 递归搜索源文件
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.*" )

# 定义要忽略的文件或文件夹
set(IGNORE_FILES
  "SourceCode/CabbageEditor/CabbageEditorBackend/*"
  "SourceCode/CabbageEditor/CabbageEditorFrontend/*")

# 过滤掉要忽略的文件或文件夹
foreach(file ${IGNORE_FILES})
  list(FILTER SOURCE_FILES EXCLUDE REGEX ${file})
endforeach()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})# 创建可执行目标，使用项目名称作为目标名
set_source_groups("${SOURCE_FILES}" ${SOURCE_DIR} "SourceCode")


# -------------------------------------------------------------------------------
# 编译器选项配置
# -------------------------------------------------------------------------------
target_compile_options(${PROJECT_NAME} PRIVATE

    # RelWithDebInfo 配置下保留调试信息:
    $<$<AND:$<CONFIG:RelWithDebInfo>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fno-omit-frame-pointer> # 保留帧指针
    $<$<AND:$<CONFIG:RelWithDebInfo>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-g> # 生成调试符号

    # MinSizeRel 配置下的优化选项:
    $<$<AND:$<CONFIG:MinSizeRel>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-ffunction-sections> # 函数分段(便于链接优化)
    $<$<AND:$<CONFIG:MinSizeRel>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fdata-sections> # 数据分段(便于链接优化)
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:Clang>>:-Oz> # Clang 最佳体积优化
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/GL> # MSVC 全程序优化

    # MSVC 通用选项:
    $<$<CXX_COMPILER_ID:MSVC>:/W1> # 警告等级 1
    $<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8> # 源文件编码 UTF-8
    $<$<CXX_COMPILER_ID:MSVC>:/execution-charset:utf-8> # 执行字符集 UTF-8
)

# -------------------------------------------------------------------------------
# 链接器选项配置
# -------------------------------------------------------------------------------
target_link_options(${PROJECT_NAME} PRIVATE

    # MinSizeRel 配置下的体积优化:
    $<$<AND:$<CONFIG:MinSizeRel>,$<PLATFORM_ID:Linux>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:LINKER:--gc-sections> # 移除未使用段
    $<$<AND:$<CONFIG:MinSizeRel>,$<PLATFORM_ID:Darwin>>:-dead_strip> # macOS 死代码消除
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/LTCG> # 链接时代码生成
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/OPT:REF> # 移除未引用数据
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/OPT:ICF> # 相同 COMDAT 折叠
)

# -------------------------------------------------------------------------------
# 预处理器定义
# -------------------------------------------------------------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE

    # 根据配置设置不同的功能开关:
    $<$<CONFIG:RelWithDebInfo>:ENABLE_DETAILED_LOGGING=1> # 启用详细日志
    $<$<CONFIG:RelWithDebInfo>:CABBAGE_ENGINE_DEBUG> # 调试模式宏 CABBAGE_ENGINE_DEBUG
    $<$<CONFIG:MinSizeRel>:DISABLE_EXTRA_FEATURES=1> # 禁用额外功能
    $<$<CONFIG:MinSizeRel>:CABBAGE_ENGINE_RELEASE> # 发布模式宏 CABBAGE_ENGINE_RELEASE

    # MSVC 特定定义:
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS> # 禁用安全警告
    NOMINMAX # 禁用 MSVC 的 min/max 宏
)

# -------------------------------------------------------------------------------
# 目标属性配置
# -------------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG> # 可执行文件输出路径
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG> # 库文件输出路径
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG> # 静态库输出路径
    PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG> # PDB 文件输出路径
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}> # VS 调试工作目录
)

# -------------------------------------------------------------------------------
# 全局属性配置
# -------------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # 启用 IDE 中的文件夹组织
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}) # 设置 VS 启动项目

# -------------------------------------------------------------------------------
# 子仓库配置
# -------------------------------------------------------------------------------
set(BUILD_SHARED_LIBS OFF)

# --- GLFW ---
add_subdirectory(ExternalEnvironment/glfw EXCLUDE_FROM_ALL)

# --- Assimp ---
add_subdirectory(ExternalEnvironment/assimp EXCLUDE_FROM_ALL)

# --- glslang ---
set(ENABLE_OPT OFF) # 通常用于关闭 glslang 的优化，可能用于调试或加速编译。
add_subdirectory(ExternalEnvironment/vulkan/glslang EXCLUDE_FROM_ALL)

# --- SPIRV-Cross ---
set(SPIRV_CROSS_SHARED OFF) # 显式要求 SPIRV-Cross 不构建共享库（与全局 BUILD_SHARED_LIBS 一致）。
set(SPIRV_CROSS_STATIC ON) # 显式要求 SPIRV-Cross 构建静态库。
set(SPIRV_CROSS_ENABLE_TESTS OFF) # 禁用测试目标，可以减少配置和编译时间。
set(SPIRV_CROSS_CLI OFF) # 禁用命令行接口工具的构建。
add_subdirectory(ExternalEnvironment/vulkan/SPIRV-Cross EXCLUDE_FROM_ALL)

# --- 链接库到主项目 ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    assimp
    glslang
    glslang-default-resource-limits # glslang 的一个组件
    spirv-cross-core
    spirv-cross-c
    spirv-cross-cpp
    spirv-cross-util
)

# -------------------------------------------------------------------------------
# Windows 平台配置
# -------------------------------------------------------------------------------
if(WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
        "${Python3_INCLUDE_DIRS}"
        "${CMAKE_CURRENT_SOURCE_DIR}/SourceCode"

        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/ktm"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/stb"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/glfw/include"

        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/boost/pfr/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/boost/preprocessor/include"

        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/vulkan"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/vulkan/volk"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/vulkan/glslang"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/vulkan/SPIRV-Cross"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/vulkan/Vulkan-Headers/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/vulkan/VulkanMemoryAllocator/include"

    )

    target_link_directories(${PROJECT_NAME} PRIVATE "${Python3_LIBRARY_DIRS}")
    target_link_libraries(${PROJECT_NAME} PRIVATE "${Python3_LIBRARIES}")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/scripts/npm_build.py"
        VERBATIM
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/ExternalEnvironment/scripts/sync_res.py"
        "--python-root-dir" "${Python3_ROOT_DIR}"
        "--runtime-output_dir" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        "--cmake-root-dir" "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
    )
endif()
