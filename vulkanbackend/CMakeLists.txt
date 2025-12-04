add_library(VulkanBackend STATIC
	VulkanContext.cpp
	VulkanMemory.cpp
	VulkanSwapchain.cpp
	VulkanBuffer.cpp
	VulkanShaderModule.cpp
	VulkanCommandPool.cpp
	VulkanRenderPass.cpp
	VulkanImage.cpp
	VulkanFramebuffer.cpp
	VulkanDescriptorSetLayout.cpp 
	VulkanPipelineLayout.cpp 
)

target_include_directories(VulkanBackend
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

target_link_libraries(VulkanBackend
	PUBLIC
		Vulkan::Vulkan
		glfw
		GPUOpen::VulkanMemoryAllocator
)