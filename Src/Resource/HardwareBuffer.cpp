#include"CabbageFramework.h"
#include"HardwareContext.h"


std::unordered_map<uint32_t, ResourceManager::ImageHardwareWrap> bufferGlobalPool;
uint32_t currentBufferID = 0;

//HardwareBuffer::operator bool()
//{
//	return buffer.bufferHandle != VK_NULL_HANDLE;
//}

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

uint32_t HardwareBuffer::storeDescriptor()
{
	return globalHardwareContext.resourceManager.storeDescriptor(buffer);
}

bool copyFromData(const void* inputData, uint64_t size)
{
	memcpy(buffer.bufferAllocInfo.pMappedData, inputData, size);
	return true;
}

template<typename Type>
bool copyFromVector(const std::vector<Type>& input)
{
	memcpy(buffer.bufferAllocInfo.pMappedData, input.data(), input.size() * sizeof(Type));
	return true;
}

void* getMappedData()
{
	return buffer.bufferAllocInfo.pMappedData;
}

uint64_t getBufferSize()
{
	return buffer.bufferAllocInfo.size;
}

HardwareBuffer& operator= (const HardwareBuffer& other)
{
	this->buffer = other.buffer;
	return *this;
}