#include"CabbageFramework.h"
#include"HardwareContext.h"


std::unordered_map<uint64_t, ResourceManager::BufferHardwareWrap> bufferGlobalPool;
uint64_t currentBufferID = 0;


HardwareBuffer& HardwareBuffer::operator= (const HardwareBuffer& other)
{
	*(this->bufferID) = *(other.bufferID);
	return *this;
}

HardwareBuffer::HardwareBuffer()
{
	this->bufferID = (uint64_t*)malloc(sizeof(uint64_t));
}


HardwareBuffer::~HardwareBuffer()
{
	//free(this->bufferID);
}

HardwareBuffer::operator bool()
{
	return bufferGlobalPool[*bufferID].bufferHandle != VK_NULL_HANDLE;
}

HardwareBuffer::HardwareBuffer(uint64_t bufferSize, BufferUsage usage, const void* data)
{
	this->bufferID = (uint64_t*)malloc(sizeof(uint64_t));
	*(this->bufferID) = currentBufferID++;

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

	bufferGlobalPool[*bufferID] = globalHardwareContext.resourceManager.createBuffer(bufferSize, vkUsage);

	if (data != nullptr)
	{
		copyFromData(data, bufferSize);
	}
}

bool HardwareBuffer::copyFromBuffer(const HardwareBuffer& inputBuffer, uint64_t size)
{
	globalHardwareContext.resourceManager.copyBuffer(bufferGlobalPool[*inputBuffer.bufferID].bufferHandle, bufferGlobalPool[*bufferID].bufferHandle, size);
	return true;
}

uint32_t HardwareBuffer::storeDescriptor()
{
	return globalHardwareContext.resourceManager.storeDescriptor(bufferGlobalPool[*bufferID]);
}

bool HardwareBuffer::copyFromData(const void* inputData, uint64_t size)
{
	memcpy(bufferGlobalPool[*bufferID].bufferAllocInfo.pMappedData, inputData, size);
	return true;
}


void* HardwareBuffer::getMappedData()
{
	return bufferGlobalPool[*bufferID].bufferAllocInfo.pMappedData;
}

uint64_t HardwareBuffer::getBufferSize()
{
	return bufferGlobalPool[*bufferID].bufferAllocInfo.size;
}
