add_library(Assets STATIC
	ObjLoader.cpp
)

target_include_directories(Assets
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Assets
	PUBLIC
		Renderer
		tinyobjloader
)