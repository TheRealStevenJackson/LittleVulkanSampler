find_package(fastgltf CONFIG REQUIRED)

add_library(Assets STATIC
	types/Mesh.cpp
	types/Material.cpp
	types/Texture.cpp
	types/Shader.cpp
	loader/MeshLoader.cpp
	loader/ImageLoader.cpp
	loader/MaterialLoader.cpp
	loader/ShaderLoader.cpp
	loader/SceneLoader.cpp
	AssetManager.cpp
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
		VulkanBackend
		tinyobjloader
	PRIVATE
		fastgltf::fastgltf
)
