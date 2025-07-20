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
    *this->pushConstantID = currentPushConstantID++;
    pushConstantGlobalPool[*this->pushConstantID] = PushConstant();
    pushConstantGlobalPool[*this->pushConstantID].size = 0;
    pushConstantGlobalPool[*this->pushConstantID].data = nullptr;
}

HardwarePushConstant::~HardwarePushConstant()
{
    //if (pushConstantGlobalPool.find(*pushConstantID) != pushConstantGlobalPool.end())
    //{
    //    free(pushConstantGlobalPool[*pushConstantID].data);
    //    pushConstantGlobalPool.erase(*pushConstantID);
    //}
    //free(pushConstantID);
}

HardwarePushConstant &HardwarePushConstant::operator=(const HardwarePushConstant &other)
{
    PushConstant &otherPC = pushConstantGlobalPool[*other.pushConstantID];
    PushConstant &selfPC = pushConstantGlobalPool[*this->pushConstantID];

    if (selfPC.size == otherPC.size && selfPC.data != nullptr && otherPC.data != nullptr)
    {
        memcpy(selfPC.data, otherPC.data, otherPC.size);
    }

    *(this->pushConstantID) = *(other.pushConstantID);

    return *this;
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole)
{
    this->pushConstantID = (uint64_t *)malloc(sizeof(uint64_t));
    *this->pushConstantID = currentPushConstantID++;
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
    PushConstant pc;
    pc.size = size;
    pc.data = (uint8_t*)malloc(size);
    memcpy(pc.data, src, size);
    pushConstantGlobalPool[*pushConstantID] = pc;
}