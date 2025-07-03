# ============================= CabbageHardware CMake 配置 =============================
cmake_minimum_required(VERSION 4.0)
set(CMAKE_POLICY_VERSION_MINIMUM "4.0")

# ======================== 项目基本信息 ========================
project(
    CabbageHardware
    VERSION 0.5.0
    HOMEPAGE_URL "https://github.com/CoronaEngine/CabbageHardware"
    LANGUAGES C CXX
)

# ======================== 检查 CMake 预设配置 ========================
if(NOT USED_PRESET_CONFIG)
    message(FATAL_ERROR "[CabbageHardware] 错误: 必须使用 CMake 预设配置 (USED_PRESET_CONFIG)！")
else()
    message(STATUS "[CabbageHardware] 检测到 USED_PRESET_CONFIG，继续配置。")
endif()

# ======================== 根目录判定与日志输出 ========================
set(IS_ROOT_DIR FALSE)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IS_ROOT_DIR TRUE)
    message(STATUS "[CabbageHardware] 当前为项目根目录: ${CMAKE_CURRENT_SOURCE_DIR}")
else()
    message(STATUS "[CabbageHardware] 当前为子目录: ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# ======================== CMake 策略配置 ========================
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW) # 使 option() 支持目录作用域
    message(STATUS "[CabbageHardware] 已启用策略 CMP0077 (option() 支持目录作用域)")
endif()

# ======================== 文件分组函数（用于 IDE 文件夹结构） ========================
function(set_source_groups SOURCE_FILES SOURCE_DIR SOURCE_DIR_NAME)
    foreach(each_source_file IN LISTS SOURCE_FILES)
        get_filename_component(each_source_path "${each_source_file}" PATH)
        file(RELATIVE_PATH each_source_path_rel ${SOURCE_DIR} ${each_source_path})

        # 空路径直接分组到根
        if("${each_source_path_rel}" STREQUAL "")
            source_group("${SOURCE_DIR_NAME}" FILES ${each_source_file})
            continue()
        endif()

        string(REPLACE "/" "\\" each_source_group_path ${each_source_path_rel})
        source_group("${SOURCE_DIR_NAME}/${each_source_group_path}" FILES ${each_source_file})
    endforeach()
endfunction()

# ======================== 源码收集 ========================
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Src)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.*")
message(STATUS "[CabbageHardware] 已收集源文件数量: ${SOURCE_FILES}")

# ======================== 忽略特定文件/文件夹 ========================
set(IGNORE_FILES "")

if(NOT IS_ROOT_DIR)
    set(IGNORE_FILES "Src/main.cpp")
    message(STATUS "[CabbageHardware] 非根目录，忽略 main.cpp")
endif()

foreach(file ${IGNORE_FILES})
    list(FILTER SOURCE_FILES EXCLUDE REGEX ${file})
endforeach()

# ======================== 主目标配置 ========================
if(IS_ROOT_DIR)
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
    message(STATUS "[CabbageHardware] 配置为可执行程序 (executable)")
else()
    add_library(${PROJECT_NAME} ${SOURCE_FILES})
    message(STATUS "[CabbageHardware] 配置为静态库 (library)")
endif()

set_source_groups("${SOURCE_FILES}" ${SOURCE_DIR} "Src")

# ======================== 编译器选项 ========================
target_compile_options(${PROJECT_NAME} PRIVATE

    # RelWithDebInfo: 保留调试信息
    $<$<AND:$<CONFIG:RelWithDebInfo>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fno-omit-frame-pointer>
    $<$<AND:$<CONFIG:RelWithDebInfo>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-g>

    # MinSizeRel: 优化体积
    $<$<AND:$<CONFIG:MinSizeRel>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-ffunction-sections>
    $<$<AND:$<CONFIG:MinSizeRel>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fdata-sections>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:Clang>>:-Oz>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/GL>

    # MSVC 通用
    $<$<CXX_COMPILER_ID:MSVC>:/W1>
    $<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>
    $<$<CXX_COMPILER_ID:MSVC>:/execution-charset:utf-8>
)

# ======================== 链接器选项 ========================
target_link_options(${PROJECT_NAME} PRIVATE

    # MinSizeRel: 链接优化
    $<$<AND:$<CONFIG:MinSizeRel>,$<PLATFORM_ID:Linux>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:LINKER:--gc-sections>
    $<$<AND:$<CONFIG:MinSizeRel>,$<PLATFORM_ID:Darwin>>:-dead_strip>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/LTCG>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/OPT:REF>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/OPT:ICF>
)

# ======================== 预处理器定义 ========================
target_compile_definitions(${PROJECT_NAME} PRIVATE

    # 配置相关
    $<$<CONFIG:RelWithDebInfo>:ENABLE_DETAILED_LOGGING=1>
    $<$<CONFIG:RelWithDebInfo>:CABBAGE_ENGINE_DEBUG>
    $<$<CONFIG:MinSizeRel>:DISABLE_EXTRA_FEATURES=1>
    $<$<CONFIG:MinSizeRel>:CABBAGE_ENGINE_RELEASE>

    # MSVC 特定
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    NOMINMAX
)

# ======================== 目标属性 ========================
if(IS_ROOT_DIR)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/$<CONFIG>
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib/$<CONFIG>
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib/$<CONFIG>
        PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/$<CONFIG>
        VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib/$<CONFIG>
        PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/$<CONFIG>
        VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# ======================== 全局属性 ========================
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# ======================== 子仓库与第三方库 ========================
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(Env/Helicon EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} PRIVATE Helicon)

if(IS_ROOT_DIR)
    add_subdirectory(Env/glfw EXCLUDE_FROM_ALL)
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
endif()

# ======================== Windows 平台专用配置 ========================
if(WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Src"
        "${CMAKE_CURRENT_SOURCE_DIR}/Env/Helicon/Src"
        "${CMAKE_CURRENT_SOURCE_DIR}/Env/ktm"
        "${CMAKE_CURRENT_SOURCE_DIR}/Env/glfw/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Env/vulkan"
        "${CMAKE_CURRENT_SOURCE_DIR}/Env/vulkan/volk"
        "${CMAKE_CURRENT_SOURCE_DIR}/Env/vulkan/Vulkan-Headers/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Env/vulkan/VulkanMemoryAllocator/include"
    )
    message(STATUS "[CabbageHardware] Windows 平台 include 路径已配置")
endif()

# ======================== 配置完成日志 ========================
if(IS_ROOT_DIR)
    message(STATUS "[CabbageHardware] 主目标与第三方库配置完成")
else()
    message(STATUS "[CabbageHardware] 子目标与第三方库配置完成")
endif()