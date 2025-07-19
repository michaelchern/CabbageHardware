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
	this->pushConstantID = (uint64_t*)malloc(sizeof(uint64_t));
}

HardwarePushConstant::~HardwarePushConstant()
{
}

HardwarePushConstant& HardwarePushConstant::operator= (const HardwarePushConstant& other)
{
	*(this->pushConstantID) = *(other.pushConstantID);
	if (this->pushConstantID != nullptr)
	{
        memcpy(this->pushConstantID, other.pushConstantID, *(this->pushConstantID));
	}
	else
	{
        this->pushConstantID = other.pushConstantID;
	}
	return *this;
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole)
{
    this->pushConstantID = (uint64_t *)malloc(sizeof(uint64_t));
    *(this->pushConstantID) = currentPushConstantID++;
	if (whole != nullptr)
	{
        pushConstantGlobalPool[*pushConstantID] = PushConstant();
        pushConstantGlobalPool[*pushConstantID].size = (uint64_t*)size ;
        pushConstantGlobalPool[*pushConstantID].data = (uint8_t *)offset;
	}
	else
	{
        pushConstantGlobalPool[*pushConstantID].data = (uint8_t *)malloc(size);
	}
}