add_library(RenderEngine STATIC
	RenderEngine.cpp
)

target_include_directories(RenderEngine
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RenderEngine
	PUBLIC
		VulkanBackend
		Platform
)