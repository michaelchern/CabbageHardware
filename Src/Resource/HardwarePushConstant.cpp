#include "CabbageFramework.h"
#include "HardwareContext.h"

struct PushConstant
{
    uint8_t *data = nullptr;
    uint64_t size = 0;
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
	return *this;
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole)
{
    this->pushConstantID = (uint64_t *)malloc(sizeof(uint64_t));
    *(this->pushConstantID) = currentPushConstantID++;
	if (whole != nullptr)
	{
        pushConstantGlobalPool[*pushConstantID] = PushConstant();
        pushConstantGlobalPool[*pushConstantID].size = (uint64_t)size ;
        pushConstantGlobalPool[*pushConstantID].data = (uint8_t *)(pushConstantGlobalPool[*(whole->pushConstantID)].data + offset);
	}
	else
	{
        pushConstantGlobalPool[*pushConstantID].data = (uint8_t *)malloc(size);
	}
}

uint8_t* HardwarePushConstant::getData() const
{
	if (pushConstantGlobalPool.count(*pushConstantID))
	{
		return pushConstantGlobalPool[*pushConstantID].data;
	}
	return nullptr;
}

void HardwarePushConstant::copyFromRaw(const void* src, uint64_t size)
{
    pushConstantID = (uint64_t *)malloc(sizeof(uint64_t));
    PushConstant pc;
    pc.size = size;
    pc.data = new uint8_t[size];
    memcpy(pc.data, src, size);
    pushConstantGlobalPool[*pushConstantID] = pc;
}