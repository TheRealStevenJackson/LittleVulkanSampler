#define STB_IMAGE_IMPLEMENTATION
#include "core/asset/loader/ImageLoader.h"

#include "platform/graphics/vulkan/VulkanImage.h"
#include "platform/graphics/vulkan/VulkanMemory.h"

#include <stb_image.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>

ImageLoader::ImageLoader(VulkanContext& context)
	: mContext(context)
{
}

VkSampler ImageLoader::createSampler(VulkanContext& context)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	
	// Query physical device properties for max anisotropy
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(context.physicalDevice(), &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkSampler sampler = VK_NULL_HANDLE;
	if (vkCreateSampler(context.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		std::cerr << "Failed to create texture sampler" << std::endl;
		return VK_NULL_HANDLE;
	}

	return sampler;
}

LoadedImage ImageLoader::loadImage(const std::string& filepath)
{
	LoadedImage result;
	
	if (!std::filesystem::exists(filepath)) {
		std::cerr << "Image file not found: " << filepath << std::endl;
		return result;
	}

	int width, height, channels;
	stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) {
		std::cerr << "Failed to load image: " << filepath << " - " << stbi_failure_reason() << std::endl;
		return result;
	}

	const VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * height * 4;
	VkExtent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	auto image = std::make_unique<VulkanImage>(mContext, format, extent, usage);
	VkImage vkImage = image->image();

	auto& vma = mContext.vma();
	VkDevice device = mContext.device();
	VkQueue queue = mContext.graphicsQueue();
	uint32_t queueFamily = mContext.findQueueFamilies(mContext.physicalDevice()).graphics_family.value();

	// Staging buffer
	VulkanMemory::AllocatedBuffer staging = vma.createBuffer(
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	vma.uploadToBuffer(staging, pixels, imageSize);
	stbi_image_free(pixels);

	// One-time command pool and buffer
	VkCommandPool cmdPool = VK_NULL_HANDLE;
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	poolInfo.queueFamilyIndex = queueFamily;
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
		vma.destroyBuffer(staging);
		std::cerr << "Failed to create command pool for image upload" << std::endl;
		return result;
	}

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = cmdPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS) {
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		std::cerr << "Failed to allocate command buffer for image upload" << std::endl;
		return result;
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
		vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		std::cerr << "Failed to begin command buffer for image upload" << std::endl;
		return result;
	}

	// UNDEFINED -> TRANSFER_DST_OPTIMAL
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = vkImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
	                     0, 0, nullptr, 0, nullptr, 1, &barrier);

	// Copy buffer -> image
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { extent.width, extent.height, 1 };
	vkCmdCopyBufferToImage(cmd, staging.buffer, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	// TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	                     0, 0, nullptr, 0, nullptr, 1, &barrier);

	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
		vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		std::cerr << "Failed to end command buffer for image upload" << std::endl;
		return result;
	}

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	if (vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE) != VK_SUCCESS) {
		vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		std::cerr << "Failed to submit image upload" << std::endl;
		return result;
	}
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
	vkDestroyCommandPool(device, cmdPool, nullptr);
	vma.destroyBuffer(staging);

	// Create sampler
	VkSampler sampler = createSampler(mContext);
	if (sampler == VK_NULL_HANDLE) {
		std::cerr << "Failed to create sampler for image: " << filepath << std::endl;
		return result;
	}

	// Populate result
	result.image = std::move(image);
	result.imageView = result.image->imageView();
	result.sampler = sampler;
	result.device = device;

	std::cout << "Image uploaded to GPU: " << filepath << " (" << width << "x" << height << ")" << std::endl;
	return result;
}

LoadedImage ImageLoader::loadImageFromMemory(const uint8_t* data, size_t size)
{
	LoadedImage result;
	if (!data || size == 0)
		return result;

	int width, height, channels;
	stbi_uc* pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) {
		std::cerr << "Failed to load image from memory - " << stbi_failure_reason() << std::endl;
		return result;
	}

	const VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * height * 4;
	VkExtent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	auto image = std::make_unique<VulkanImage>(mContext, format, extent, usage);
	VkImage vkImage = image->image();

	auto& vma = mContext.vma();
	VkDevice device = mContext.device();
	VkQueue queue = mContext.graphicsQueue();
	uint32_t queueFamily = mContext.findQueueFamilies(mContext.physicalDevice()).graphics_family.value();

	VulkanMemory::AllocatedBuffer staging = vma.createBuffer(
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	vma.uploadToBuffer(staging, pixels, imageSize);
	stbi_image_free(pixels);

	VkCommandPool cmdPool = VK_NULL_HANDLE;
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	poolInfo.queueFamilyIndex = queueFamily;
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
		vma.destroyBuffer(staging);
		return result;
	}

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = cmdPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS) {
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		return result;
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
		vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		return result;
	}

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = vkImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
	                     0, 0, nullptr, 0, nullptr, 1, &barrier);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { extent.width, extent.height, 1 };
	vkCmdCopyBufferToImage(cmd, staging.buffer, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	                     0, 0, nullptr, 0, nullptr, 1, &barrier);

	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
		vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		return result;
	}

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	if (vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE) != VK_SUCCESS) {
		vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		return result;
	}
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
	vkDestroyCommandPool(device, cmdPool, nullptr);
	vma.destroyBuffer(staging);

	VkSampler sampler = createSampler(mContext);
	if (sampler == VK_NULL_HANDLE)
		return result;

	result.image = std::move(image);
	result.imageView = result.image->imageView();
	result.sampler = sampler;
	result.device = device;
	return result;
}

std::unique_ptr<VulkanImage> ImageLoader::createWhiteTexture()
{
	// Create a 1x1 white texture (RGBA = 1.0, 1.0, 1.0, 1.0)
	const VkExtent2D extent{ 1, 1 };
	const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	
	auto image = std::make_unique<VulkanImage>(mContext, format, extent, usage);
	VkImage vkImage = image->image();
	
	auto& vma = mContext.vma();
	VkDevice device = mContext.device();
	VkQueue queue = mContext.graphicsQueue();
	uint32_t queueFamily = mContext.findQueueFamilies(mContext.physicalDevice()).graphics_family.value();
	
	// 1x1 RGBA pixel data (white = 255, 255, 255, 255)
	const uint8_t whitePixel[4] = { 255, 255, 255, 255 };
	const VkDeviceSize imageSize = 4;
	
	// Staging buffer
	VulkanMemory::AllocatedBuffer staging = vma.createBuffer(
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	vma.uploadToBuffer(staging, whitePixel, imageSize);
	
	// One-time command pool and buffer
	VkCommandPool cmdPool = VK_NULL_HANDLE;
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	poolInfo.queueFamilyIndex = queueFamily;
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
		vma.destroyBuffer(staging);
		std::cerr << "Failed to create command pool for white texture upload" << std::endl;
		return nullptr;
	}
	
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = cmdPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS) {
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		std::cerr << "Failed to allocate command buffer for white texture upload" << std::endl;
		return nullptr;
	}
	
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
		vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		std::cerr << "Failed to begin command buffer for white texture upload" << std::endl;
		return nullptr;
	}
	
	// UNDEFINED -> TRANSFER_DST_OPTIMAL
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = vkImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
	                     0, 0, nullptr, 0, nullptr, 1, &barrier);
	
	// Copy buffer -> image
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { extent.width, extent.height, 1 };
	vkCmdCopyBufferToImage(cmd, staging.buffer, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	
	// TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	                     0, 0, nullptr, 0, nullptr, 1, &barrier);
	
	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
		vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		std::cerr << "Failed to end command buffer for white texture upload" << std::endl;
		return nullptr;
	}
	
	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	if (vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE) != VK_SUCCESS) {
		vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
		vkDestroyCommandPool(device, cmdPool, nullptr);
		vma.destroyBuffer(staging);
		std::cerr << "Failed to submit white texture upload" << std::endl;
		return nullptr;
	}
	vkQueueWaitIdle(queue);
	
	vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
	vkDestroyCommandPool(device, cmdPool, nullptr);
	vma.destroyBuffer(staging);
	
	return image;
}
