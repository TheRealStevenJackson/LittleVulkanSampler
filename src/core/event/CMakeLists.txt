add_library(CoreEvent STATIC
	InputEventQueue.cpp
	InputManager.cpp
)

target_include_directories(CoreEvent
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/../..
)
