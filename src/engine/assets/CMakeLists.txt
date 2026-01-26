add_library(Assets STATIC
	ObjLoader.cpp
	AssetManager.cpp
	StbLoader.cpp
)

target_include_directories(Assets
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/../..
)

find_package(Stb REQUIRED)

target_include_directories(Assets
	PUBLIC
		${Stb_INCLUDE_DIR}
)

target_link_libraries(Assets
	PUBLIC
		Renderer
		tinyobjloader
)