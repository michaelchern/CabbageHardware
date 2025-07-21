#include"CabbageFramework.h"
#include<HardwareManager/GlobalContext.h>

struct PushConstant
{
    uint8_t *data = nullptr;
    uint64_t size = 0;
};

std::unordered_map<uint64_t, PushConstant> pushConstantGlobalPool;
std::unordered_map<uint64_t, uint64_t> pushConstantRefCount;
uint64_t currentPushConstantID = 0;


HardwarePushConstant::HardwarePushConstant()
{
	this->pushConstantID = (uint64_t*)malloc(sizeof(uint64_t));
    *this->pushConstantID = currentPushConstantID++;

    pushConstantRefCount[*this->pushConstantID] = 1;

    pushConstantGlobalPool[*this->pushConstantID] = PushConstant();
    pushConstantGlobalPool[*this->pushConstantID].size = 0;
    pushConstantGlobalPool[*this->pushConstantID].data = nullptr;
}

HardwarePushConstant::~HardwarePushConstant()
{
    if (pushConstantRefCount[*this->pushConstantID] == 1)
    {
        free(pushConstantGlobalPool[*this->pushConstantID].data);
        pushConstantGlobalPool.erase(*this->pushConstantID);
        pushConstantRefCount.erase(*this->pushConstantID);
    }
    else if (pushConstantRefCount[*this->pushConstantID] > 1)
    {
        pushConstantRefCount[*this->pushConstantID]--;
    }
    free(this->pushConstantID);
}

HardwarePushConstant &HardwarePushConstant::operator=(const HardwarePushConstant &other)
{   
    pushConstantRefCount[*this->pushConstantID]++;

    if (pushConstantGlobalPool[*this->pushConstantID].size == pushConstantGlobalPool[*other.pushConstantID].size && pushConstantGlobalPool[*this->pushConstantID].data != nullptr && pushConstantGlobalPool[*other.pushConstantID].data != nullptr)
    {
        memcpy(pushConstantGlobalPool[*this->pushConstantID].data, pushConstantGlobalPool[*other.pushConstantID].data, pushConstantGlobalPool[*other.pushConstantID].size);
    }

    *this->pushConstantID = *(other.pushConstantID);
    return *this;
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole)
{
    this->pushConstantID = (uint64_t *)malloc(sizeof(uint64_t));
    *this->pushConstantID = currentPushConstantID++;

    pushConstantRefCount[*this->pushConstantID] = 1;

    pushConstantGlobalPool[*this->pushConstantID] = PushConstant();
    pushConstantGlobalPool[*this->pushConstantID].size = size;
    if (whole != nullptr)
    {
        pushConstantGlobalPool[*this->pushConstantID].data = pushConstantGlobalPool[*(whole->pushConstantID)].data + offset;
    }
    else
    {
        pushConstantGlobalPool[*this->pushConstantID].data = (uint8_t *)malloc(size);
    }
}

uint8_t* HardwarePushConstant::getData() const
{
    return pushConstantGlobalPool.at(*pushConstantID).data;
}

uint64_t HardwarePushConstant::getSize() const
{
    return pushConstantGlobalPool.at(*pushConstantID).size;
}

void HardwarePushConstant::copyFromRaw(const void* src, uint64_t size)
{
    pushConstantID = (uint64_t *)malloc(sizeof(uint64_t));
    *this->pushConstantID = currentPushConstantID++;

    pushConstantRefCount[*this->pushConstantID] = 1;

    PushConstant pushConstant;
    pushConstant.size = size;
    pushConstant.data = (uint8_t *)malloc(size);
    memcpy(pushConstant.data, src, size);
    pushConstantGlobalPool[*pushConstantID] = pushConstant;
}