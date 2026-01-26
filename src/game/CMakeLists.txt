# Game layer: components and game logic above the engine.
add_library(Game INTERFACE)

target_include_directories(Game INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Game INTERFACE
	Assets
)
