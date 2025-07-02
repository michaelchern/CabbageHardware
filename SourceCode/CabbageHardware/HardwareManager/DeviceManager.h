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
#include<Volk/volk.h>

class DeviceManager
{
public:

	struct DeviceFeaturesChain
	{
	public:
		DeviceFeaturesChain();

		VkPhysicalDeviceFeatures2* getChainHead();

		DeviceFeaturesChain operator& (DeviceFeaturesChain features);
		DeviceFeaturesChain operator& (VkPhysicalDeviceFeatures features);
		DeviceFeaturesChain operator& (VkPhysicalDeviceFeatures2 features);
		DeviceFeaturesChain operator& (VkPhysicalDeviceVulkan11Features features);
		DeviceFeaturesChain operator& (VkPhysicalDeviceVulkan12Features features);
		DeviceFeaturesChain operator& (VkPhysicalDeviceVulkan13Features features);
		DeviceFeaturesChain operator& (VkPhysicalDeviceAccelerationStructureFeaturesKHR features);
		DeviceFeaturesChain operator& (VkPhysicalDeviceRayTracingPipelineFeaturesKHR features);
		DeviceFeaturesChain operator& (VkPhysicalDeviceRayQueryFeaturesKHR features);




		DeviceFeaturesChain operator| (DeviceFeaturesChain features);
		DeviceFeaturesChain operator| (VkPhysicalDeviceFeatures features);
		DeviceFeaturesChain operator| (VkPhysicalDeviceFeatures2 features);
		DeviceFeaturesChain operator| (VkPhysicalDeviceVulkan11Features features);
		DeviceFeaturesChain operator| (VkPhysicalDeviceVulkan12Features features);
		DeviceFeaturesChain operator| (VkPhysicalDeviceVulkan13Features features);
		DeviceFeaturesChain operator| (VkPhysicalDeviceAccelerationStructureFeaturesKHR features);
		DeviceFeaturesChain operator| (VkPhysicalDeviceRayTracingPipelineFeaturesKHR features);
		DeviceFeaturesChain operator| (VkPhysicalDeviceRayQueryFeaturesKHR features);



	private:
		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		VkPhysicalDeviceVulkan11Features deviceFeatures11{};
		VkPhysicalDeviceVulkan12Features deviceFeatures12{};
		VkPhysicalDeviceVulkan13Features deviceFeatures13{};

		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
		VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};

		friend DeviceFeaturesChain;
	};


	struct CreateCallback
	{
		std::function<std::set<const char*>(const VkInstance& instance, const VkPhysicalDevice& device)> requiredInstanceExtensions =
			[](const VkInstance& instance, const VkPhysicalDevice& device) {return std::set<const char*>(); };

		std::function<std::set<const char*>(const VkInstance& instance, const VkPhysicalDevice& device)> requiredDeviceExtensions =
			[](const VkInstance& instance, const VkPhysicalDevice& device) {return std::set<const char*>(); };

		std::function<DeviceFeaturesChain(const VkInstance& instance, const VkPhysicalDevice& device)> requiredDeviceFeatures =
			[](const VkInstance& instance, const VkPhysicalDevice& device) {return DeviceFeaturesChain(); };
	};



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
	};



	DeviceManager();

	~DeviceManager();

	void initDeviceManager(const CreateCallback& createCallback);

	VkInstance getVulkanInstance();

	void createTimelineSemaphore();


	//bool executeSingleTimeCommands(std::function<void(VkCommandBuffer& commandBuffer)> commandsFunction);

	bool executeSingleTimeCommands(std::function<void(VkCommandBuffer& commandBuffer)> commandsFunction);

	DeviceUtils mainDevice;

	std::vector<DeviceUtils> userDevices;

private:

	void createVkInstance(const CreateCallback& createInfo);

	void createDevices(const CreateCallback& createInfo);

	void chooseMainDevice();

	void choosePresentQueueFamily();

	bool createCommandBuffers();

	VkInstance vkInstance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

	uint64_t semaphoreValue = 0;
	VkSemaphore timelineSemaphore;

};