#pragma once

#include<iostream>
#include<set>
#include<stdexcept>
#include<vector>
#include<array>
#include<optional>
#include<functional>

#if defined(_WIN32)
#   define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__) || defined(__unix__)
#   define VK_USE_PLATFORM_XLIB_KHR
#elif defined(__APPLE__)
#   define VK_USE_PLATFORM_MACOS_MVK
#else
#   error "Platform not supported by this example."
#endif

#define VK_NO_PROTOTYPES
#include<volk/volk.h>

#include"FeaturesChain.h"


class DeviceManager
{
public:

	struct DeviceUtils
	{
		struct QueueUtils
		{
			VkQueue vkQueue = VK_NULL_HANDLE;
			uint32_t queueFamilyIndex = -1;
		};

		std::vector<QueueUtils> graphicsQueues;
		std::vector<QueueUtils> computeQueues;
		std::vector<QueueUtils> transferQueues;

		struct FeaturesUtils
		{
			std::set<const char*> instanceExtensions{};
			std::set<const char*> deviceExtensions{};

			VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties{};
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{};
			VkPhysicalDeviceProperties2 supportedProperties{};

			DeviceFeaturesChain featuresChain{};
		}deviceFeaturesUtils;

		//bool checkDeviceSupport(FeaturesUtils required);

		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkCommandBuffer commandBuffers = VK_NULL_HANDLE;

		std::vector<VkQueueFamilyProperties> queueFamilies;

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice logicalDevice = VK_NULL_HANDLE;

		bool operator== (DeviceUtils& other)
		{
			return physicalDevice == other.physicalDevice && logicalDevice == other.logicalDevice;
		}

		uint64_t semaphoreValue = 0;
        VkSemaphore timelineSemaphore;
	};



	DeviceManager();

	~DeviceManager();

	void initDeviceManager(const CreateCallback &createCallback, const VkInstance& vkInstance);


	void createTimelineSemaphore();


	//bool executeSingleTimeCommands(std::function<void(VkCommandBuffer& commandBuffer)> commandsFunction);

	bool executeSingleTimeCommands(std::function<void(VkCommandBuffer& commandBuffer)> commandsFunction);

	DeviceUtils mainDevice;

	std::vector<DeviceUtils> userDevices;

private:

	void createDevices(const CreateCallback &createInfo, const VkInstance &vkInstance);

	void chooseMainDevice();

	void choosePresentQueueFamily();

	bool createCommandBuffers();
};