add_executable(SpinningCube
	SpinningCube.cpp
)

target_link_libraries(SpinningCube
	PRIVATE
		Platform
		VulkanBackend
)