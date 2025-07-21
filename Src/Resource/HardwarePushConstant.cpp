#include"CabbageFramework.h"
#include<Hardware/GlobalContext.h>

struct PushConstant
{
    uint8_t *data = nullptr;
    uint64_t size = 0;
    bool isSub = false;
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
    pushConstantRefCount[*this->pushConstantID]--;
    if (pushConstantRefCount[*this->pushConstantID] == 0)
    {
        if (pushConstantGlobalPool[*this->pushConstantID].data != nullptr)
        {
            //free(pushConstantGlobalPool[*this->pushConstantID].data);
            //pushConstantGlobalPool[*this->pushConstantID].data = nullptr;
        }
        //pushConstantGlobalPool.erase(*this->pushConstantID);
        //pushConstantRefCount.erase(*this->pushConstantID);
    }
}

HardwarePushConstant &HardwarePushConstant::operator=(const HardwarePushConstant &other)
{   
    if (pushConstantGlobalPool[*this->pushConstantID].size != pushConstantGlobalPool[*other.pushConstantID].size || pushConstantGlobalPool[*this->pushConstantID].data == nullptr)
    {
        if (pushConstantGlobalPool[*this->pushConstantID].data != nullptr && pushConstantGlobalPool[*this->pushConstantID].isSub != true)
        {
            free(pushConstantGlobalPool[*this->pushConstantID].data);
        }
        pushConstantGlobalPool[*this->pushConstantID].size = pushConstantGlobalPool[*other.pushConstantID].size;
        pushConstantGlobalPool[*this->pushConstantID].data = (uint8_t *)malloc(pushConstantGlobalPool[*other.pushConstantID].size);
    }

    if (pushConstantGlobalPool[*other.pushConstantID].data != nullptr)
    {
        memcpy(pushConstantGlobalPool[*this->pushConstantID].data, pushConstantGlobalPool[*other.pushConstantID].data, pushConstantGlobalPool[*other.pushConstantID].size);
    }

    pushConstantRefCount[*this->pushConstantID]++; 
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
        pushConstantGlobalPool[*this->pushConstantID].isSub = true;
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