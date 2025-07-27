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

    std::vector<VkSemaphore> imageAvailableSemaphores;

    VkSemaphore timelineSemaphore = VK_NULL_HANDLE;
    uint64_t timelineCounter = 0;
    std::vector<uint64_t> frameTimelineValues;
    
    std::atomic_uint16_t currentQueueIndex = 0;
    std::vector<DeviceManager::QueueUtils> presentQueues;

    std::shared_ptr<HardwareContext::HardwareUtils> displayDevice;


	void cleaarupDisplayManager();


	void createVkSurface(void* surface);

	void choosePresentDevice();


	void createSyncObjects();

	void createSwapChain();

	bool submitQueuePresent(VkPresentInfoKHR &persentInfo)
    {
        DeviceManager::QueueUtils *queue;
        uint16_t queueIndex = 0;

        while (true)
        {
            uint16_t queueIndex = currentQueueIndex.fetch_add(1) % presentQueues.size();
            queue = &presentQueues[queueIndex];

            if (queue->queueMutex->try_lock())
            {
                uint64_t timelineCounterValue = 0;
                vkGetSemaphoreCounterValue(displayDevice->deviceManager.logicalDevice, queue->timelineSemaphore, &timelineCounterValue);
                if (timelineCounterValue >= queue->timelineValue)
                {
                    break;
                }
                else
                {
                    queue->queueMutex->unlock();
                }
            }

            std::this_thread::yield();
        }

        //std::cout << "Present Queue Index: " << queueIndex << std::endl;

        VkResult result = vkQueuePresentKHR(queue->vkQueue, &persentInfo);

        queue->queueMutex->unlock();

		if (result != VK_SUCCESS)
		{
            throw std::runtime_error("failed to vkQueuePresentKHR for a frame!");
		}
        return true;
    }

	uint32_t currentFrame = 0;
};