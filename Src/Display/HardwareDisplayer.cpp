#include"CabbageDisplayer.h"
#include"DisplayManager.h"

std::unordered_map<void *, DisplayManager> displayerGlobalPool;


HardwareDisplayer::HardwareDisplayer(void* surface): surface(surface)
{
    if (surface != nullptr && !displayerGlobalPool.count(surface))
    {
        displayerGlobalPool[surface] = DisplayManager();
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
    if (displayerGlobalPool.count(surface))
    {
        displayerGlobalPool[surface].displayFrame(surface,image);
    }
    return *this;
}
