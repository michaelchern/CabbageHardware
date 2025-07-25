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

private:

    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    ktm::uvec2 displaySize = ktm::uvec2(0, 0);
    void *displaySurface = nullptr;

    std::vector<ResourceManager::ImageHardwareWrap> swapChainImages;

	VkSurfaceFormatKHR surfaceFormat;


	ResourceManager::ImageHardwareWrap displayImage;

    std::vector<VkSemaphore> swapchainSemaphore;

    std::vector<DeviceManager::QueueUtils> presentQueues;
    HardwareContext::HardwareUtils *displayDevice;


	void cleaarupDisplayManager();


	void createVkSurface(void* surface);

	void choosePresentDevice();


	void createFrameSemaphores();

	void createSwapChain();

	bool submitQueuePresent(VkPresentInfoKHR &persentInfo)
    {
        uint16_t queueIndex = 0;
        while (true)
        {
            if (presentQueues[queueIndex].queueMutex->try_lock())
            {
                break;
            }
            else
            {
                queueIndex = (queueIndex + 1) % presentQueues.size();
            }
        }

        VkResult result = vkQueuePresentKHR(presentQueues[queueIndex].vkQueue, &persentInfo);

        presentQueues[queueIndex].queueMutex->unlock();

		if (result != VK_SUCCESS)
		{
            throw std::runtime_error("failed to vkQueuePresentKHR for a frame!");
		}
        return true;
    }

	uint32_t currentFrame = 0;
};