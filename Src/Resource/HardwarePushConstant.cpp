#include"CabbageFramework.h"
#include"HardwareContext.h"

HardwarePushConstant::HardwarePushConstant()
{
	pushConstantSize = (uint64_t*)malloc(sizeof(uint64_t));
}

HardwarePushConstant::~HardwarePushConstant()
{
}

HardwarePushConstant& HardwarePushConstant::operator= (const HardwarePushConstant& other)
{
	this->pushConstantData = other.pushConstantData;
	*(this->pushConstantSize) = *(other.pushConstantSize);
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