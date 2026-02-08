# CMakeList.txt : CMake project for FirstVulkanEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)
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

add_subdirectory(src/core/asset)
add_subdirectory(external)
add_subdirectory(src/platform)
add_subdirectory(src/core/event)
add_subdirectory(src/game)
add_subdirectory(src/engine/input)
add_subdirectory(src/platform/graphics/vulkan)
add_subdirectory(src/engine/graphics)
add_subdirectory(samples)
