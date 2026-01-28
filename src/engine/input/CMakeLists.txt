add_library(EngineInput STATIC
	Controller.cpp
)

target_include_directories(EngineInput
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(EngineInput
	PUBLIC
		CoreEvent
)
