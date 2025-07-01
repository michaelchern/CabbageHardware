#pragma once

#include <chrono>  

#include"ResourceManager.h"

#include"DeviceManager.h"



class DisplayManager
{
public:

	DisplayManager();
	~DisplayManager();

	bool initDisplayManager(void* surface);

	bool displayFrame(void *surface, ResourceManager::ImageHardwareWrap &displayImage);


	static std::set<const char*> checkInstanceExtensionRequirements(const VkInstance& instance, const VkPhysicalDevice& device);
	static std::set<const char*> checkDeviceExtensionRequirements(const VkInstance& instance, const VkPhysicalDevice& device);


	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;

	ktm::uvec2 displaySize = ktm::uvec2(0, 0);
	void *displaySurface = nullptr;

	std::vector<ResourceManager::ImageHardwareWrap> swapChainImages;
	std::vector<VkSemaphore> swapchainSemaphore;

	std::vector<DeviceManager::DeviceUtils::QueueUtils> presentQueues;
	DeviceManager::DeviceUtils displayDevice;

//private:
	VkSurfaceFormatKHR surfaceFormat;

	void cleaarupDisplayManager();


	void createVkSurface(void* surface);

	void choosePresentDevice();


	void createFrameFence();

	void createSwapChain();

	uint32_t currentFrame = 0;
};