#include"CabbageFramework.h"



HardwareBuffer::HardwareBuffer(uint64_t bufferSize, BufferUsage usage, const void* data)
{
	VkBufferUsageFlags vkUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	switch (usage)
	{
	case BufferUsage::VertexBuffer:
		vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		break;
	case BufferUsage::IndexBuffer:
		vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;
	case BufferUsage::UniformBuffer:
		vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	case BufferUsage::StorageBuffer:
		vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	default:
		break;
	}

	buffer = globalHardwareContext.resourceManager.createBuffer(bufferSize, vkUsage);
	if (data != nullptr)
	{
		copyFromData(data, bufferSize);
	}
}

bool HardwareBuffer::copyFromBuffer(const HardwareBuffer& inputBuffer, uint64_t size)
{
	globalHardwareContext.resourceManager.copyBuffer(inputBuffer.buffer.bufferHandle, buffer.bufferHandle, size);
	return true;
}


HardwareImage::HardwareImage(ktm::uvec2 imageSize, ImageFormat imageFormat, ImageUsage imageUsage, int arrayLayers, void* imageData)
{
	VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	switch (imageUsage)
	{
	case ImageUsage::SampledImage:
		vkImageUsageFlags = vkImageUsageFlags | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		break;
	case ImageUsage::StorageImage:
		vkImageUsageFlags = vkImageUsageFlags | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		break;
	case ImageUsage::DepthImage:
		vkImageUsageFlags = vkImageUsageFlags | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	default:
		break;
	}

	uint32_t pixelSize;
	VkFormat vkImageFormat;
	switch (imageFormat)
	{
	case ImageFormat::RGBA8_UINT:
		vkImageFormat = VK_FORMAT_R8G8B8A8_UINT;
		pixelSize = 8 * 4 / 8;
		break;
	case ImageFormat::RGBA8_SINT:
		vkImageFormat = VK_FORMAT_R8G8B8A8_SINT;
		pixelSize = 8 * 4 / 8;
		break;
	case ImageFormat::RGBA8_SRGB:
		vkImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		pixelSize = 8 * 4 / 8;
		break;
	case ImageFormat::RGBA16_UINT:
		vkImageFormat = VK_FORMAT_R16G16B16A16_UINT;
		pixelSize = 16 * 4 / 8;
		break;
	case ImageFormat::RGBA16_SINT:
		vkImageFormat = VK_FORMAT_R16G16B16A16_SINT;
		pixelSize = 16 * 4 / 8;
		break;
	case ImageFormat::RGBA16_FLOAT:
		vkImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		pixelSize = 16 * 4 / 8;
		break;
	case ImageFormat::RGBA32_UINT:
		vkImageFormat = VK_FORMAT_R32G32B32A32_UINT;
		pixelSize = 32 * 4 / 8;
		break;
	case ImageFormat::RGBA32_SINT:
		vkImageFormat = VK_FORMAT_R32G32B32A32_SINT;
		pixelSize = 32 * 4 / 8;
		break;
	case ImageFormat::RGBA32_FLOAT:
		vkImageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		pixelSize = 32 * 4 / 8;
		break;
	case ImageFormat::RG32_FLOAT:
		vkImageFormat = VK_FORMAT_R32G32_SFLOAT;
		pixelSize = 32 * 2 / 8;
		break;
	case ImageFormat::D16_UNORM:
		vkImageFormat = VK_FORMAT_D16_UNORM;
		pixelSize = 16 / 8;
		break;
	case ImageFormat::D32_FLOAT:
		vkImageFormat = VK_FORMAT_D32_SFLOAT;
		pixelSize = 32 / 8;
		break;
	default:
		break;
	}

	image = globalHardwareContext.resourceManager.createImage(imageSize, vkImageFormat, vkImageUsageFlags, arrayLayers);
	image.pixelSize = pixelSize;

	if (imageData != nullptr)
	{
		copyFromData(imageData);
	}
}

HardwareImage::operator bool()
{
	return image.imageHandle != VK_NULL_HANDLE;
}

uint32_t HardwareBuffer::storeDescriptor()
{
	return globalHardwareContext.resourceManager.storeDescriptor(buffer);
}

uint32_t HardwareImage::storeDescriptor()
{
	return globalHardwareContext.resourceManager.storeDescriptor(image);
}


bool HardwareImage::copyFromBuffer(const HardwareBuffer& buffer)
{
	globalHardwareContext.resourceManager.copyBufferToImage(buffer.buffer.bufferHandle, image.imageHandle, image.imageSize.x,image.imageSize.y);
	return true;
}

bool HardwareImage::copyFromData(const void* inputData)
{
	HardwareBuffer stagingBuffer = HardwareBuffer(image.imageSize.x * image.imageSize.y * image.pixelSize, BufferUsage::StorageBuffer, inputData);
	copyFromBuffer(stagingBuffer);
	return true;
}
