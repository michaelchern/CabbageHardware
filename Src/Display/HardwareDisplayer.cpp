#include"CabbageDisplayer.h"
#include"DisplayManager.h"

std::unordered_map < void *, std::shared_ptr<DisplayManager>> displayerGlobalPool;

std::mutex displayerMutex;


HardwareDisplayer::HardwareDisplayer(void* surface): surface(surface)
{
    if (surface != nullptr && !displayerGlobalPool.count(surface))
    {
        std::unique_lock<std::mutex> lock(displayerMutex);
        displayerGlobalPool[surface] = std::make_shared<DisplayManager>();
    }
}

HardwareDisplayer::~HardwareDisplayer()
{
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareDisplayer &other)
{
    this->surface = other.surface;
    return *this;
}

HardwareDisplayer& HardwareDisplayer::operator = (const HardwareImage& image)
{
    std::unique_lock<std::mutex> lock(displayerMutex);
    if (displayerGlobalPool.count(surface))
    {
        displayerGlobalPool[surface]->displayFrame(surface,image);
    }
    return *this;
}
