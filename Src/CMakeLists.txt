file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
add_library(CabbageHardware STATIC ${SOURCE_FILES})

set_target_properties(CabbageHardware PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/$<CONFIG>
)

target_include_directories(CabbageHardware PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/Env/Helicon/Src"
    "${PROJECT_SOURCE_DIR}/Env/ktm"
    "${PROJECT_SOURCE_DIR}/Env/vulkan"
    "${PROJECT_SOURCE_DIR}/Env/vulkan/volk"
    "${PROJECT_SOURCE_DIR}/Env/vulkan/Vulkan-Headers/include"
    "${PROJECT_SOURCE_DIR}/Env/vulkan/VulkanMemoryAllocator/include"
)

target_link_libraries(CabbageHardware PUBLIC Helicon)