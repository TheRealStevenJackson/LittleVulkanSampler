add_library(EngineScene STATIC
	SceneManager.cpp
)

target_include_directories(EngineScene
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(EngineScene
	PUBLIC
		Assets
		Game
)
