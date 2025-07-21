#pragma once

#include <chrono>  

#include<Hardware/GlobalContext.h>

#include<CabbageDisplayer.h>


class DisplayManager
{
public:

	DisplayManager();
	~DisplayManager();

	bool initDisplayManager(void* surface);

	bool displayFrame(void *surface, HardwareImage displayImage);

	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;

	ktm::uvec2 displaySize = ktm::uvec2(0, 0);
	void *displaySurface = nullptr;

	std::vector<ResourceManager::ImageHardwareWrap> swapChainImages;
	std::vector<VkSemaphore> swapchainSemaphore;

	std::vector<DeviceManager::DeviceUtils::QueueUtils> presentQueues;
    HardwareContext::HardwareUtils *displayDevice;

//private:
	VkSurfaceFormatKHR surfaceFormat;

	void cleaarupDisplayManager();


	void createVkSurface(void* surface);

	void choosePresentDevice();


	void createFrameFence();

	void createSwapChain();

	uint32_t currentFrame = 0;
};