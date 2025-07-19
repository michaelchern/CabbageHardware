#pragma once

#include "HardwareManager/DeviceManager.h"
#include "HardwareManager/ResourceManager.h"

struct HardwareContext
{
    HardwareContext();

    ~HardwareContext() = default;

    DeviceManager deviceManager;
    ResourceManager resourceManager;
    // std::vector<DisplayManager> displayManagers;
};

extern HardwareContext globalHardwareContext;

extern std::unordered_map<uint64_t, ResourceManager::ImageHardwareWrap> imageGlobalPool;
extern std::unordered_map<uint64_t, ResourceManager::BufferHardwareWrap> bufferGlobalPool;