#include"CabbageFramework.h"
#include"HardwareContext.h"

struct PushConstant
{
    uint8_t *data = nullptr;
    uint64_t *size = nullptr;
};

std::unordered_map<uint64_t, PushConstant> pushConstantGlobalPool;
uint64_t currentPushConstantID = 0;


HardwarePushConstant::HardwarePushConstant()
{
	pushConstantSize = (uint64_t*)malloc(sizeof(uint64_t));
}

HardwarePushConstant::~HardwarePushConstant()
{
}

HardwarePushConstant& HardwarePushConstant::operator= (const HardwarePushConstant& other)
{
	*(this->pushConstantSize) = *(other.pushConstantSize);
	if (this->pushConstantData != nullptr)
	{
		memcpy(this->pushConstantData, other.pushConstantData, *(this->pushConstantSize));
	}
	else
	{
		this->pushConstantData = other.pushConstantData;
	}
	return *this;
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole)
{
	pushConstantSize = (uint64_t*)malloc(sizeof(uint64_t));
	*pushConstantSize = size;
	if (whole != nullptr)
	{
		pushConstantData = (uint8_t*)(whole->pushConstantData) + offset;
	}
	else
	{
		pushConstantData = (uint8_t*)malloc(size);
	}
}