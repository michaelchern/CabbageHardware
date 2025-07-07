file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
add_library(CabbageDisplay STATIC ${SOURCE_FILES})

set_target_properties(CabbageDisplay PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/$<CONFIG>
)

target_compile_options(CabbageDisplay PRIVATE
    $<$<AND:$<CONFIG:RelWithDebInfo>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fno-omit-frame-pointer -g>
    $<$<AND:$<CONFIG:MinSizeRel>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-ffunction-sections -fdata-sections>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:Clang>>:-Oz>
    $<$<AND:$<CONFIG:MinSizeRel>,$<CXX_COMPILER_ID:MSVC>>:/GL>
    $<$<CXX_COMPILER_ID:MSVC>:/W1 /source-charset:utf-8 /execution-charset:utf-8>
)

target_compile_definitions(CabbageDisplay PRIVATE
    $<$<CONFIG:RelWithDebInfo>:ENABLE_DETAILED_LOGGING=1>
    $<$<CONFIG:RelWithDebInfo>:CABBAGE_ENGINE_DEBUG>
    $<$<CONFIG:MinSizeRel>:DISABLE_EXTRA_FEATURES=1>
    $<$<CONFIG:MinSizeRel>:CABBAGE_ENGINE_RELEASE>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    NOMINMAX
)

target_include_directories(CabbageDisplay PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/Env/Helicon/Src"
    "${PROJECT_SOURCE_DIR}/Env/ktm"
    "${PROJECT_SOURCE_DIR}/Env/vulkan"
    "${PROJECT_SOURCE_DIR}/Env/vulkan/volk"
    "${PROJECT_SOURCE_DIR}/Env/vulkan/Vulkan-Headers/include"
    "${PROJECT_SOURCE_DIR}/Env/vulkan/VulkanMemoryAllocator/include"
)

target_link_libraries(CabbageDisplay PUBLIC Helicon)