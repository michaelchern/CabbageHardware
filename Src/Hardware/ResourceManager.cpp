#include"ResourceManager.h"

//#define VMA_STATS_STRING_ENABLED 0
#define VK_NO_PROTOTYPES
#define VMA_IMPLEMENTATION
#include<vk_mem_alloc.h>


#include<Hardware/GlobalContext.h>


ResourceManager::ResourceManager()
{
}


void ResourceManager::initResourceManager(DeviceManager &device)
{
    this->device = &device;

	CreateVmaAllocator();
	createTextureSampler();
	createBindlessDescriptorSet();
}



void ResourceManager::CreateVmaAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};

    std::vector<VkExternalMemoryHandleTypeFlagsKHR> externalMemoryHandleTypes;
#if _WIN32 || _WIN64
	externalMemoryHandleTypes.push_back(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT);
#endif
    allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes.data();

	bool g_EnableValidationLayer = true;
	bool VK_KHR_get_memory_requirements2_enabled = false;
	bool VK_KHR_get_physical_device_properties2_enabled = false;
	bool VK_KHR_dedicated_allocation_enabled = false;
	bool VK_KHR_bind_memory2_enabled = false;
	bool VK_EXT_memory_budget_enabled = false;
	bool VK_AMD_device_coherent_memory_enabled = false;
	bool VK_KHR_buffer_device_address_enabled = true;
	bool VK_EXT_memory_priority_enabled = false;
	bool VK_EXT_debug_utils_enabled = false;
	bool VK_KHR_maintenance5_enabled = false;
	bool g_SparseBindingEnabled = false;

	allocatorInfo.physicalDevice = this->device->deviceUtils.physicalDevice;
    allocatorInfo.device = this->device->deviceUtils.logicalDevice;
    allocatorInfo.instance = globalHardwareContext.getVulkanInstance();
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;

	if (VK_KHR_dedicated_allocation_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
	}
	if (VK_KHR_bind_memory2_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
	}
#if !defined(VMA_MEMORY_BUDGET) || VMA_MEMORY_BUDGET == 1
	if (VK_EXT_memory_budget_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	}
#endif
	if (VK_AMD_device_coherent_memory_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
	}
	if (VK_KHR_buffer_device_address_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}
#if !defined(VMA_MEMORY_PRIORITY) || VMA_MEMORY_PRIORITY == 1
	if (VK_EXT_memory_priority_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
	}
#endif
	if (VK_KHR_maintenance5_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
	}

	//if (USE_CUSTOM_CPU_ALLOCATION_CALLBACKS)
	//{
	//	allocatorInfo.pAllocationCallbacks = &g_CpuAllocationCallbacks;
	//}

#if VMA_DYNAMIC_VULKAN_FUNCTIONS
	static VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	allocatorInfo.pVulkanFunctions = &vulkanFunctions;
#endif

	// Uncomment to enable recording to CSV file.
	/*
	static VmaRecordSettings recordSettings = {};
	recordSettings.pFilePath = "VulkanSample.csv";
	outInfo.pRecordSettings = &recordSettings;
	*/

	// Uncomment to enable HeapSizeLimit.
	/*
	static std::array<VkDeviceSize, VK_MAX_MEMORY_HEAPS> heapSizeLimit;
	std::fill(heapSizeLimit.begin(), heapSizeLimit.end(), VK_WHOLE_SIZE);
	heapSizeLimit[0] = 512ull * 1024 * 1024;
	outInfo.pHeapSizeLimit = heapSizeLimit.data();
	*/
	vmaCreateAllocator(&allocatorInfo, &g_hAllocator);


	const VkPhysicalDeviceMemoryProperties* memProps = nullptr;
	vmaGetMemoryProperties(g_hAllocator, &memProps);
	for (uint32_t heapIndex = 0; heapIndex < memProps->memoryHeapCount; ++heapIndex)
	{
		const VkMemoryHeap& heap = memProps->memoryHeaps[heapIndex];
		if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
		{
			deviceMemorySize += heap.size;
		}
		else if (heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT)
		{
			mutiInstanceMemorySize += heap.size;
		}
		else
		{
			hostSharedMemorySize += heap.size;
		}
	}
}


void ResourceManager::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = false;
    samplerInfo.maxAnisotropy = this->device->deviceUtils.deviceFeaturesUtils.supportedProperties.properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(1);
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(this->device->deviceUtils.logicalDevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void ResourceManager::destroyBuffer(BufferHardwareWrap& buffer)
{
	vmaDestroyBuffer(g_hAllocator, buffer.bufferHandle, buffer.bufferAlloc);
}


ResourceManager::BufferHardwareWrap  ResourceManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage)
{
	BufferHardwareWrap resultBuffer;
    resultBuffer.device = this->device;
    resultBuffer.resourceManager = this;

	if (size > 0)
	{
		resultBuffer.bufferUsage = usage;

		VkBufferCreateInfo vbInfo = {};
		vbInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vbInfo.size = size;
		vbInfo.usage = usage;
		vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vbAllocCreateInfo = {};
		vbAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		vbAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		vmaCreateBuffer(g_hAllocator, &vbInfo, &vbAllocCreateInfo, &resultBuffer.bufferHandle, &resultBuffer.bufferAlloc, &resultBuffer.bufferAllocInfo);
	}

	return resultBuffer;
}

void ResourceManager::destroyImage(ImageHardwareWrap& image)
{
	if (image.imageView != VK_NULL_HANDLE)
	{
        vkDestroyImageView(this->device->deviceUtils.logicalDevice, image.imageView, nullptr);
	}
	if (image.imageAlloc != VK_NULL_HANDLE && image.imageHandle != VK_NULL_HANDLE)
	{
		vmaDestroyImage(g_hAllocator, image.imageHandle, image.imageAlloc);
	}
}


VkImageView ResourceManager::createImageView(ImageHardwareWrap& image)
{
	// Check if the format supports the required usage
	//VkFormatProperties2 formatProperties = {};
	//vkGetPhysicalDeviceFormatProperties2(deviceManager.mainDevice.physicalDevice, image.imageFormat, &formatProperties);

	//if ((image.imageUsage & VK_IMAGE_USAGE_STORAGE_BIT) && !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
	//	throw std::runtime_error("Image format does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT.");
	//}

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image.imageHandle;
	viewInfo.viewType = image.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = image.imageFormat;
	viewInfo.subresourceRange.aspectMask = image.aspectMask;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = image.mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = image.arrayLayers;
	viewInfo.flags = 0;

	if (vkCreateImageView(this->device->deviceUtils.logicalDevice, &viewInfo, nullptr, &image.imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
		return VK_NULL_HANDLE;
	}
	else
	{
		return image.imageView;
	}
}

ResourceManager::ImageHardwareWrap ResourceManager::createImage(ktm::uvec2 imageSize, VkFormat imageFormat, VkImageUsageFlags imageUsage, int arrayLayers, int mipLevels)
{
	ImageHardwareWrap resultImage;

    resultImage.device = this->device;
    resultImage.resourceManager = this;

	resultImage.imageSize = imageSize;
	resultImage.imageFormat = imageFormat;
	resultImage.arrayLayers = arrayLayers;
	resultImage.mipLevels = mipLevels;
	resultImage.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// Check if the format and usage are supported
	//VkImageFormatProperties2 imageFormatProperties = {};
	//VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
	//imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
	//imageFormatInfo.format = imageFormat;
	//imageFormatInfo.type = VK_IMAGE_TYPE_2D;
	//imageFormatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	//imageFormatInfo.usage = imageUsage;
	//imageFormatInfo.flags = 0;

	//if (vkGetPhysicalDeviceImageFormatProperties2(deviceManager.mainDevice.physicalDevice, &imageFormatInfo, &imageFormatProperties) != VK_SUCCESS) {
	//	throw std::runtime_error("Image format not supported for the given usage.");
	//}

	if (imageUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		resultImage.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		resultImage.clearValue.depthStencil = { 1.0f, 0 };
	}
	else
	{
		resultImage.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resultImage.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	}

	imageUsage = imageUsage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	resultImage.imageUsage = imageUsage;


	if (imageSize.x != 0 || imageSize.y != 0)
	{
        VkExternalMemoryImageCreateInfo externalInfo = {};
        externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
#if _WIN32 || _WIN64
        externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT; // Windows下句柄类型
#elif __APPLE__
#elif __linux__
#endif

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = imageSize.x;
		imageInfo.extent.height = imageSize.y;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = imageFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = imageUsage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.pNext = &externalInfo;

		VmaAllocationCreateInfo imageAllocCreateInfo = {};
		imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		vmaCreateImage(g_hAllocator, &imageInfo, &imageAllocCreateInfo, &resultImage.imageHandle, &resultImage.imageAlloc, &resultImage.imageAllocInfo);

		resultImage.imageView = createImageView(resultImage);
	}

	return resultImage;
}

bool ResourceManager::copyImageMemory(ImageHardwareWrap& source, ImageHardwareWrap& destination)
{
    if (source.imageSize != destination.imageSize)
    {
        return false;
    }

    if (source.device != destination.device)
    {
        VkDeviceSize imageSizeBytes = source.imageSize.x * source.imageSize.y * 4; // 需根据format实际计算

        // 1. 在source device上创建host可访问staging buffer
        ResourceManager::BufferHardwareWrap srcStaging = source.resourceManager->createBuffer(imageSizeBytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        // 2. 拷贝source image到staging buffer
        auto srcCopyCmd = [&](VkCommandBuffer commandBuffer) {
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = source.aspectMask;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {source.imageSize.x, source.imageSize.y, 1};
            vkCmdCopyImageToBuffer(commandBuffer, source.imageHandle, VK_IMAGE_LAYOUT_GENERAL, srcStaging.bufferHandle, 1, &region);
        };
        source.device->executeSingleTimeCommands(srcCopyCmd);

        // 3. 映射staging buffer，读取数据
        void *mappedData = nullptr;
        vmaMapMemory(g_hAllocator, srcStaging.bufferAlloc, &mappedData);

        // 4. 在destination device上创建host可访问staging buffer
        ResourceManager::BufferHardwareWrap dstStaging = destination.resourceManager->createBuffer(imageSizeBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        // 5. 映射destination staging buffer，写入数据
        void *dstMappedData = nullptr;
        vmaMapMemory(g_hAllocator, dstStaging.bufferAlloc, &dstMappedData);
        memcpy(dstMappedData, mappedData, imageSizeBytes);
        vmaUnmapMemory(g_hAllocator, srcStaging.bufferAlloc);
        vmaUnmapMemory(g_hAllocator, dstStaging.bufferAlloc);

        // 6. 拷贝destination staging buffer到destination image
        auto dstCopyCmd = [&](VkCommandBuffer commandBuffer) {
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = destination.aspectMask;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {destination.imageSize.x, destination.imageSize.y, 1};
            vkCmdCopyBufferToImage(commandBuffer, dstStaging.bufferHandle, destination.imageHandle, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
        };
        destination.device->executeSingleTimeCommands(dstCopyCmd);

        // 7. 销毁staging buffer
        destroyBuffer(srcStaging);
        destroyBuffer(dstStaging);

        return true;
    }
    else
	{
		auto runCommand = [&](VkCommandBuffer& commandBuffer)
			{
				VkImageCopy imageCopyRegion{};
				imageCopyRegion.srcSubresource.aspectMask = source.aspectMask;
				imageCopyRegion.srcSubresource.layerCount = 1;
				imageCopyRegion.dstSubresource.aspectMask = destination.aspectMask;
				imageCopyRegion.dstSubresource.layerCount = 1;
				imageCopyRegion.extent.width = source.imageSize.x;
				imageCopyRegion.extent.height = source.imageSize.y;
				imageCopyRegion.extent.depth = 1;

				vkCmdCopyImage(commandBuffer, source.imageHandle, VK_IMAGE_LAYOUT_GENERAL, destination.imageHandle, VK_IMAGE_LAYOUT_GENERAL, 1, &imageCopyRegion);
			};

		globalHardwareContext.mainDevice->deviceManager.executeSingleTimeCommands(runCommand);

		return true;
	}
}


void ResourceManager::transitionImageLayoutUnblocked(VkCommandBuffer &commandBuffer, ImageHardwareWrap &image,
                                                     VkImageLayout newLayout, VkPipelineStageFlags sourceStage,
                                                     VkPipelineStageFlags destinationStage)
{
    if (image.imageLayout != newLayout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = image.imageLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image.imageHandle;
        barrier.subresourceRange.aspectMask = image.aspectMask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = image.mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = image.arrayLayers;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                 newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
                 newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
                 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        image.imageLayout = newLayout;
    }
}



void ResourceManager::transitionImageLayout(ImageHardwareWrap& image, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
{
	if (image.imageLayout != newLayout)
	{
		auto runCommand = [&](VkCommandBuffer& commandBuffer)
			{
				transitionImageLayoutUnblocked(commandBuffer, image, newLayout, sourceStage, destinationStage);
			};

		globalHardwareContext.mainDevice->deviceManager.executeSingleTimeCommands(runCommand);
	}
}


void ResourceManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	auto runCommand = [&](VkCommandBuffer& commandBuffer)
		{
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height,
				1
			};

			vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		};

	globalHardwareContext.mainDevice->deviceManager.executeSingleTimeCommands(runCommand);
}

void ResourceManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	auto runCommand = [&](VkCommandBuffer& commandBuffer)
		{
			VkBufferCopy copyRegion{};
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		};

	globalHardwareContext.mainDevice->deviceManager.executeSingleTimeCommands(runCommand);
}


void ResourceManager::createBindlessDescriptorSet()
{
	uint32_t k_max_bindless_resources[4] = { 8,8,8,8 };

	//k_max_bindless_resources[0] = std::min(
	//	std::min(
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxPerStageResources/4,
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxPerStageDescriptorUniformBuffers),
	//	std::min(
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxDescriptorSetUniformBuffers,
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxDescriptorSetUniformBuffersDynamic)
	//);

	//k_max_bindless_resources[1] = std::min(
	//	std::min(
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxPerStageResources/4,
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxPerStageDescriptorSampledImages
	//	),
	//	deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxDescriptorSetSampledImages
	//);

	//k_max_bindless_resources[2] = std::min(
	//	std::min(
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxPerStageResources/4,
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxPerStageDescriptorStorageBuffers
	//	),
	//	std::min(
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxDescriptorSetStorageBuffers,
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxDescriptorSetStorageBuffersDynamic)
	//);

	//k_max_bindless_resources[3] = std::min(
	//	std::min(
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxPerStageResources/4,
	//		deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxPerStageDescriptorStorageImages),
	//	deviceManager.mainDevice.deviceFeaturesUtils.supportedProperties.properties.limits.maxDescriptorSetStorageImages
	//);
	//
	//std::cout << k_max_bindless_resources[0] << std::endl;
	//std::cout << k_max_bindless_resources[1] << std::endl;
	//std::cout << k_max_bindless_resources[2] << std::endl;
	//std::cout << k_max_bindless_resources[3] << std::endl;


	std::array<VkDescriptorSetLayoutBinding, 4> bindings{};
	std::array<VkDescriptorBindingFlags, 4> flags{};
	std::array<VkDescriptorType, 4> types{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
	};

	for (uint32_t i = 0; i < 4; ++i)
	{
		bindings[i].descriptorCount = k_max_bindless_resources[i];

		bindings.at(i).binding = i;
		bindings.at(i).descriptorType = types.at(i);
		// Due to partially bound bit, this value
		// is used as an upper bound, which we have set to
		// 1000 to keep it simple for the sake of this post
		//bindings.at(i).descriptorCount = 1000;
		bindings.at(i).stageFlags = VK_SHADER_STAGE_ALL;
		flags.at(i) = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}


	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
	bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	bindingFlags.pNext = nullptr;
	bindingFlags.pBindingFlags = flags.data();
	bindingFlags.bindingCount = (uint32_t)flags.size();

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = (uint32_t)bindings.size();
	createInfo.pBindings = bindings.data();
	// Create if from a descriptor pool that has update after bind
	// flag set
	createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	// Set binding flags
	createInfo.pNext = &bindingFlags;

	// Create layout
    VkResult result = vkCreateDescriptorSetLayout(this->device->deviceUtils.logicalDevice, &createInfo, nullptr, &bindlessDescriptor.descriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}




	{

		std::array<VkDescriptorPoolSize, 4> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = k_max_bindless_resources[0];
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[1].descriptorCount = k_max_bindless_resources[1];
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = k_max_bindless_resources[2];
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes[3].descriptorCount = k_max_bindless_resources[3];

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = k_max_bindless_resources[0] + k_max_bindless_resources[1] + k_max_bindless_resources[2] + k_max_bindless_resources[3];
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

		if (vkCreateDescriptorPool(this->device->deviceUtils.logicalDevice, &poolInfo, nullptr, &bindlessDescriptor.descriptorPool) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	{
		uint32_t max_binding[4] = { k_max_bindless_resources[0], k_max_bindless_resources[1] ,k_max_bindless_resources[2],k_max_bindless_resources[3] };

		VkDescriptorSetVariableDescriptorCountAllocateInfo count_info{};
		count_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		count_info.descriptorSetCount = 1;
		count_info.pDescriptorCounts = max_binding;
		count_info.pNext = nullptr;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = bindlessDescriptor.descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &bindlessDescriptor.descriptorSetLayout;
		allocInfo.pNext = &count_info;

		//uboDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        VkResult result = vkAllocateDescriptorSets(this->device->deviceUtils.logicalDevice, &allocInfo, &bindlessDescriptor.descriptorSet);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}
}



uint32_t ResourceManager::storeDescriptor(ImageHardwareWrap image)
{
	// Ensure the image view is created with the correct usage flags for storage images
	//if (image.imageUsage & VK_IMAGE_USAGE_STORAGE_BIT) {
	//	image.imageView = createImageView(image);
	//}

	uint32_t textureIndex = -1;

	VkDescriptorType descriptorType = (image.imageUsage & VK_IMAGE_USAGE_STORAGE_BIT) ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;


	if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	{
		auto it = StorageImageBindingList.find(image.imageView);
		if (it != StorageImageBindingList.end())
		{
			textureIndex = it->second;
		}
		else
		{
			textureIndex = StorageImageBindingIndex++;
			StorageImageBindingList.insert(std::pair<VkImageView, int>(image.imageView, textureIndex));
		}
	}
	else
	{
		auto it = TextureBindingList.find(image.imageView);
		if (it != TextureBindingList.end())
		{
			textureIndex = it->second;
		}
		else
		{
			textureIndex = TextureBindingIndex++;
			TextureBindingList.insert(std::pair<VkImageView, int>(image.imageView, textureIndex));
		}
	}


	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageInfo.imageView = image.imageView;
	imageInfo.sampler = textureSampler;

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = descriptorType;
	write.dstSet = bindlessDescriptor.descriptorSet;
	write.descriptorCount = 1;
	write.dstArrayElement = textureIndex;
	write.pImageInfo = &imageInfo;

	if (write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	{
		write.dstBinding = TextureBinding;
	}
	if (write.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	{
		write.dstBinding = StorageImageBinding;
	}

	vkUpdateDescriptorSets(this->device->deviceUtils.logicalDevice, 1, &write, 0, nullptr);

	return textureIndex;
}


uint32_t ResourceManager::storeDescriptor(BufferHardwareWrap buffer)
{
	uint32_t bufferIndex = -1;

	VkDescriptorType descriptorType = (buffer.bufferUsage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
	{
		auto it = StorageBufferBindingList.find(buffer.bufferHandle);
		if (it != StorageBufferBindingList.end())
		{
			bufferIndex = it->second;
		}
		else
		{
			bufferIndex = StorageBufferBindingIndex++;
			StorageBufferBindingList.insert(std::pair<VkBuffer, int>(buffer.bufferHandle, bufferIndex));
		}
	}
	else
	{
		auto it = UniformBindingList.find(buffer.bufferHandle);
		if (it != UniformBindingList.end())
		{
			bufferIndex = it->second;
		}
		else
		{
			bufferIndex = UniformBindingIndex++;
			UniformBindingList.insert(std::pair<VkBuffer, int>(buffer.bufferHandle, bufferIndex));
		}
	}



	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer.bufferHandle;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet writes{};
	writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes.dstSet = bindlessDescriptor.descriptorSet;
	writes.descriptorCount = 1;
	writes.pBufferInfo = &bufferInfo;
	writes.descriptorType = descriptorType;
	writes.dstArrayElement = bufferIndex;

	if (writes.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
	{
		writes.dstBinding = UniformBinding;
	}
	if (writes.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
	{
		writes.dstBinding = StorageBufferBinding;
	}

	vkUpdateDescriptorSets(this->device->deviceUtils.logicalDevice, 1, &writes, 0, nullptr);

	return bufferIndex;
}
//
//uint32_t ResourceManager::storeDescriptor(VkAccelerationStructureKHR m_tlas)
//{
//	uint32_t asIndex = -1;
//
//	{
//		auto it = accelerationStructureBindingList.find(m_tlas);
//		if (it != accelerationStructureBindingList.end())
//		{
//			asIndex = it->second;
//		}
//		else
//		{
//			asIndex = accelerationStructureBindingIndex++;
//			accelerationStructureBindingList.insert(std::pair<VkAccelerationStructureKHR, int>(m_tlas, asIndex));
//		}
//	}
//
//	VkWriteDescriptorSetAccelerationStructureKHR descASInfo = {};
//	descASInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
//	descASInfo.accelerationStructureCount = 1;
//	descASInfo.pAccelerationStructures = &m_tlas;
//
//	VkWriteDescriptorSet writes{};
//	writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//	writes.dstSet = bindlessDescriptor.descriptorSet;
//	writes.dstBinding = accelerationStructureBinding;
//	writes.dstArrayElement = asIndex;
//	writes.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
//	writes.descriptorCount = 1;
//	writes.pImageInfo = VK_NULL_HANDLE;
//	writes.pBufferInfo = VK_NULL_HANDLE;
//	writes.pNext = &descASInfo;
//
//	vkUpdateDescriptorSets(deviceManager.mainDevice.logicalDevice, 1, &writes, 0, nullptr);
//
//	return asIndex;
//}



VkShaderModule ResourceManager::createShaderModule(const std::vector<unsigned int>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size() * sizeof(unsigned int);
	createInfo.pCode = code.data();

	VkShaderModule shaderModule;
    if (vkCreateShaderModule(this->device->deviceUtils.logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
