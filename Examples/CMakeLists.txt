file(GLOB_RECURSE EXAMPLES_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EXAMPLES_SOURCE_FILES})
add_executable(CabbageDisplayerExamples ${EXAMPLES_SOURCE_FILES})

# 设置VS启动项目
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CabbageDisplayerExamples)

set_target_properties(CabbageDisplayerExamples PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
    PDB_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:CabbageDisplayerExamples>
)

target_include_directories(CabbageDisplayerExamples PRIVATE
    "${PROJECT_SOURCE_DIR}/Env/glfw/include"
    "${PROJECT_SOURCE_DIR}/Env/stb")

target_link_libraries(CabbageDisplayerExamples PRIVATE
    glfw
    CabbageDisplayer)