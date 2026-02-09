find_package(fastgltf CONFIG REQUIRED)

add_library(EngineScene STATIC
	SceneLoader.cpp
	SceneManager.cpp
	SceneResource.cpp
)

target_include_directories(EngineScene
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(EngineScene
	PUBLIC
		Assets
		Game
	PRIVATE
		fastgltf::fastgltf
)
