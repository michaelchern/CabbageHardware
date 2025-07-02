#pragma once

#include <type_traits>
#include <algorithm>

#include <ktm/ktm.h>

#include"CabbageFramework/CabbageCommon/ShaderCodeCompiler/ShaderCodeCompiler.h"

#include"HardwareContext.h"

//#include"CabbageFramework/PipelineManager/ComputePipeline.h"
//#include"CabbageFramework/PipelineManager/RasterizerPipeline.h"

enum class ImageFormat : uint32_t
{
	RGBA8_UINT,
	RGBA8_SINT,
	RGBA8_SRGB,

	RGBA16_UINT,
	RGBA16_SINT,
	RGBA16_FLOAT,

	RGBA32_UINT,
	RGBA32_SINT,
	RGBA32_FLOAT,

	RG32_FLOAT,

	D16_UNORM, D32_FLOAT, //just for depth
};

enum class ImageUsage : uint32_t
{
	SampledImage = 1,
	StorageImage = 2,
	DepthImage = 3,
};

enum class BufferUsage : int32_t
{
	VertexBuffer = 1,
	IndexBuffer = 2,  // must be uint32_t
	UniformBuffer = 4,
	StorageBuffer = 8,
};

template <typename Base, typename Derived>
concept NotDerivedFrom = !std::is_base_of<Base, Derived>::value;

struct HardwareResource
{
	HardwareResource() = default;
	~HardwareResource() = default;

	template<typename Type>
	HardwareResource& operator= (const Type& other) requires std::is_base_of<HardwareResource, Type>::value
	{
		this->buffer = other.buffer;
		this->image = other.image;

		this->pushConstantSize = pushConstantSize;

		if (pushConstantData != nullptr)
		{
			free(pushConstantData);
			pushConstantData = nullptr;
		}
		pushConstantData = malloc(pushConstantSize);
		memcpy(this->pushConstantData, other.pushConstantData, pushConstantSize);

		return *this;
	}

	template<typename Type>
	HardwareResource& operator= (Type data) requires NotDerivedFrom<HardwareResource, Type>
	{
		memcpy(pushConstantData, &data, pushConstantSize);
		return *this;
	}

//protected:
	friend struct HardwareBuffer;
	friend struct HardwareImage;
	friend struct HardwarePushConstant;
	friend struct HardwareRasterPipeline;
	friend struct HardwareComputePipeline;
	friend struct HardwareFrameSurface;
	ResourceManager::ImageHardwareWrap image;
	ResourceManager::BufferHardwareWrap buffer;

	void* pushConstantData = nullptr;
	uint64_t pushConstantSize = 0;


	HardwareResource(const HardwareResource& whole, uint64_t offset, uint64_t size)
	{
		pushConstantData = (uint8_t*)whole.pushConstantData + offset;
		pushConstantSize = size;
	}
};

template<typename T>
concept is_container = requires(T a)
{
	a.size();
	a.data();
	a[0];
};

struct HardwareBuffer : HardwareResource
{
	HardwareBuffer()
	{}

	//HardwareBuffer(uint32_t size, BufferUsage usage);

	//HardwareBuffer(const HardwareBuffer& inputBuffer, uint32_t bufferSize, BufferUsage usage);
	HardwareBuffer(uint64_t size, BufferUsage usage, const void* data = nullptr);

	template<typename Type>
	HardwareBuffer(const Type& input, BufferUsage usage) requires is_container<Type>
		: HardwareBuffer(input.size() * sizeof(input[0]), usage, input.data())
	{}

	~HardwareBuffer()
	{
		//globalHardwareContext.resourceManager.destroyBuffer(buffer);
	}

	uint32_t storeDescriptor();

	bool copyFromBuffer(const HardwareBuffer& inputBuffer, uint64_t size);
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

	operator bool()
	{
		return buffer.bufferHandle != VK_NULL_HANDLE;
	}
};


struct HardwareImage : HardwareResource
{
	HardwareImage()
	{}

	HardwareImage(ktm::uvec2 imageSize, ImageFormat imageFormat, ImageUsage imageUsage = ImageUsage::SampledImage, int arrayLayers = 1, void* imageData = nullptr);

	~HardwareImage() = default;

	operator bool();

	uint32_t storeDescriptor();

	bool blitFromImage(const HardwareImage& buffer);
	bool copyFromImage(const HardwareImage& buffer);
	bool copyFromBuffer(const HardwareBuffer& buffer);
	bool copyFromData(const void* inputData);

	ImageFormat imageFormat;
	ktm::uvec2 imageSize;
	uint32_t pixelSize;
};


struct HardwarePushConstant : HardwareResource
{
	HardwarePushConstant()
	{}

	HardwarePushConstant(uint64_t size)
	{
		pushConstantSize = size;
		pushConstantData = malloc(size);
	}

	HardwarePushConstant& operator= (const HardwarePushConstant& other)
	{
		pushConstantSize = other.pushConstantSize;
		if (pushConstantData != nullptr)
		{
			free(pushConstantData);
			pushConstantData = nullptr;
		}
		pushConstantData = malloc(pushConstantSize);
		memcpy(this->pushConstantData, other.pushConstantData, pushConstantSize);

		return *this;
	}

	~HardwarePushConstant() = default;
};
