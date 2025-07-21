#pragma once

#include "Hardware/DeviceManager.h"
#include "Hardware/ResourceManager.h"

struct HardwareContext
{
    HardwareContext();

    ~HardwareContext() = default;

    DeviceManager deviceManager;
    ResourceManager resourceManager;

private:
    void prepareFeaturesChain();
    CreateCallback hardwareCreateInfos{};
};

extern HardwareContext globalHardwareContext;

extern std::unordered_map<uint64_t, ResourceManager::ImageHardwareWrap> imageGlobalPool;
extern std::unordered_map<uint64_t, ResourceManager::BufferHardwareWrap> bufferGlobalPool;