add_executable(SpinningCube
	SpinningCube.cpp
)

target_link_libraries(SpinningCube
	PRIVATE
		Platform
		VulkanBackend
)

find_program(GLSLC_EXECUTABLE glslc)
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc executable not found. Please install the Vulkan SDK.")
endif()

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/spv)

file(GLOB SHADER_FILES
    ${SHADER_DIR}/*.vert
    ${SHADER_DIR}/*.frag
)

set(GLSLC_EXE "C:/VulkanSDK/1.4.321.1/Bin/glslc.exe")

set(SPIRV_OUTPUTS "")

foreach(SHADER ${SHADER_FILES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV ${SHADER_OUTPUT_DIR}/${FILE_NAME}.spv)

    message(STATUS "Registering shader: ${FILE_NAME}")

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
        COMMAND ${GLSLC_EXE} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader: ${FILE_NAME}"
        VERBATIM
    )

    list(APPEND SPIRV_OUTPUTS ${SPIRV})
endforeach()

add_custom_target(SpinningCubeShaders ALL DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(SpinningCube SpinningCubeShaders)

# Important for Visual Studio:
# Attach SPIR-V outputs as sources of the executable
set_source_files_properties(${SPIRV_OUTPUTS} PROPERTIES GENERATED TRUE)
target_sources(SpinningCube PRIVATE ${SPIRV_OUTPUTS})
