# CMakeList.txt : CMake project for FirstVulkanEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project ("FirstVulkanEngine")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(platform)
add_subdirectory(vulkanbackend)
add_subdirectory(renderengine)
add_subdirectory(samples)

add_executable (FirstVulkanEngine
	FirstVulkanEngine.cpp
)

if (WIN32)
    # Force a console subsystem so stdout/stderr appear
    set_target_properties(FirstVulkanEngine PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

target_include_directories(VulkanBackend
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FirstVulkanEngine
	PRIVATE 
		RenderEngine
		Platform
)