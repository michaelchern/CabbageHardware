#include "DeviceManager.h"

#define VOLK_IMPLEMENTATION
#include<Volk/volk.h>


DeviceManager::DeviceManager()
{
}


DeviceManager::~DeviceManager()
{

}


void DeviceManager::initDeviceManager(const CreateCallback& createCallback)
{
	if (volkInitialize() != VK_SUCCESS)
	{
		throw std::runtime_error("Failed volkInitialize!");
	}

	createVkInstance(createCallback);

	volkLoadInstance(vkInstance);

	createDevices(createCallback);

	choosePresentQueueFamily();

	chooseMainDevice();

	createCommandBuffers();

	createTimelineSemaphore();
}


void DeviceManager::createTimelineSemaphore()
{
	VkSemaphoreTypeCreateInfo type_create_info{};
	type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
	type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
	type_create_info.initialValue = 0;

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = &type_create_info;

	if (vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreInfo, nullptr, &timelineSemaphore) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create synchronization objects for a frame!");
	}
}


VkInstance DeviceManager::getVulkanInstance()
{ 
	return vkInstance; 
}


void DeviceManager::createVkInstance(const CreateCallback& initInfo)
{
	std::set<const char*> inputExtensions = initInfo.requiredInstanceExtensions(vkInstance, nullptr);

	std::vector<const char*> requiredExtensions(inputExtensions.begin(), inputExtensions.end());
	std::vector<const char*> requiredLayers{};

#ifdef CABBAGE_ENGINE_DEBUG
	bool enableDebugLayer = false;

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto& layerProperties : availableLayers)
	{
		if (strcmp("VK_LAYER_KHRONOS_validation", layerProperties.layerName) == 0)
		{
			enableDebugLayer = true;

			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
			break;
		}
	}
#endif


	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		for (size_t i = 0; i < requiredExtensions.size(); i++)
		{
			bool extensionSupported = false;
			for (size_t j = 0; j < availableExtensions.size(); j++)
			{
				if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0)
				{
					extensionSupported = true;
					break;
				}
			}
			if (!extensionSupported)
			{
				std::cerr << " ---------- Extensions Warning ---------- Instance not support : " << requiredExtensions[i] << std::endl;

				requiredExtensions.erase(requiredExtensions.begin() + i);
				i--;
			}
		}
	}


	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (size_t i = 0; i < requiredLayers.size(); i++)
		{
			bool extensionSupported = false;
			for (size_t j = 0; j < availableLayers.size(); j++)
			{
				if (strcmp(requiredLayers[i], availableLayers[j].layerName) == 0)
				{
					extensionSupported = true;
					break;
				}
			}
			if (!extensionSupported)
			{
				std::cerr << " ---------- Extensions Warning ---------- Instance not support : " << requiredLayers[i] << std::endl;

				requiredLayers.erase(requiredLayers.begin() + i);
				i--;
			}
		}
	}


	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_API_VERSION_1_4;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
	createInfo.ppEnabledLayerNames = requiredLayers.data();
	createInfo.pNext = nullptr;


#ifdef CABBAGE_ENGINE_DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableDebugLayer)
	{
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		auto debugCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)->VkBool32
		{
			std::cerr << " ---------- Vulkan Validation Layer ------ \n" << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		};

		debugCreateInfo.pfnUserCallback = debugCallback;

		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
#endif

	VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}


#ifdef CABBAGE_ENGINE_DEBUG
	if (enableDebugLayer)
	{
		auto CreateDebugUtilsMessengerEXT = [](VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)->VkResult
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		};

		if (CreateDebugUtilsMessengerEXT(vkInstance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
#endif

}


void DeviceManager::createDevices(const CreateCallback& initInfo)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

	if (deviceCount <= 0)
	{
		throw std::runtime_error("Failed to find GPUs! Please buy a GPU!");
	}

	userDevices.resize(deviceCount);
	for (uint32_t index = 0; index < deviceCount; index++)
	{
		userDevices[index].physicalDevice = devices[index];

		std::set<const char*> inputExtensions = initInfo.requiredDeviceExtensions(vkInstance, userDevices[index].physicalDevice);
		std::vector<const char*> requiredExtensions = std::vector<const char*>(inputExtensions.begin(), inputExtensions.end());

		userDevices[index].deviceFeaturesUtils.supportedProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		userDevices[index].deviceFeaturesUtils.rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		userDevices[index].deviceFeaturesUtils.accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
		userDevices[index].deviceFeaturesUtils.rayTracingPipelineProperties.pNext = &userDevices[index].deviceFeaturesUtils.accelerationStructureProperties;
		userDevices[index].deviceFeaturesUtils.supportedProperties.pNext = &userDevices[index].deviceFeaturesUtils.rayTracingPipelineProperties;
		userDevices[index].deviceFeaturesUtils.accelerationStructureProperties.pNext = nullptr;

		vkGetPhysicalDeviceProperties2(userDevices[index].physicalDevice, &userDevices[index].deviceFeaturesUtils.supportedProperties);
		std::cout << "---------- GPU " << index << " : "<< userDevices[index].deviceFeaturesUtils.supportedProperties.properties.deviceName << "----------" << std::endl;


		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(userDevices[index].physicalDevice, nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(userDevices[index].physicalDevice, nullptr, &extensionCount, availableExtensions.data());

			for (size_t i = 0; i < requiredExtensions.size(); i++)
			{
				bool extensionSupported = false;
				for (size_t j = 0; j < availableExtensions.size(); j++)
				{
					if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0)
					{
						extensionSupported = true;
						break;
					}
				}
				if (!extensionSupported)
				{
					std::cerr << "      Extensions Warning : Device not support : " << requiredExtensions[i] << std::endl;

					requiredExtensions.erase(requiredExtensions.begin() + i);
					i--;
				}
			}
		}

		
		vkGetPhysicalDeviceFeatures2(userDevices[index].physicalDevice, userDevices[index].deviceFeaturesUtils.featuresChain.getChainHead());

		userDevices[index].deviceFeaturesUtils.featuresChain = userDevices[index].deviceFeaturesUtils.featuresChain & initInfo.requiredDeviceFeatures(vkInstance, userDevices[index].physicalDevice);


		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(userDevices[index].physicalDevice, &queueFamilyCount, nullptr);
		userDevices[index].queueFamilies.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(userDevices[index].physicalDevice, &queueFamilyCount, userDevices[index].queueFamilies.data());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;


		std::vector<std::vector<float>> queuePriorities(userDevices[index].queueFamilies.size());

		for (int i = 0; i < userDevices[index].queueFamilies.size(); i++)
		{
			queuePriorities[i].resize(userDevices[index].queueFamilies[i].queueCount);
			for (int j = 0; j < queuePriorities[i].size(); j++)
			{
				queuePriorities[i][j] = 1.0f;
			}

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = i;
			queueCreateInfo.queueCount = userDevices[index].queueFamilies[i].queueCount;
			queueCreateInfo.pQueuePriorities = queuePriorities[i].data();
			queueCreateInfos.push_back(queueCreateInfo);
		}


		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.pEnabledFeatures = nullptr;
		createInfo.pNext = userDevices[index].deviceFeaturesUtils.featuresChain.getChainHead();

		VkResult result = vkCreateDevice(userDevices[index].physicalDevice, &createInfo, nullptr, &userDevices[index].logicalDevice);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device!");
		}
	}

}


void DeviceManager::choosePresentQueueFamily()
{
	for (int index = 0; index < userDevices.size(); index++)
	{
		for (int i = 0; i < userDevices[index].queueFamilies.size(); i++)
		{
			DeviceUtils::QueueUtils tempQueueUtils;
			tempQueueUtils.queueFamilyIndex = i;

			for (uint32_t queueIndex = 0; queueIndex < userDevices[index].queueFamilies[i].queueCount; queueIndex++)
			{
				vkGetDeviceQueue(userDevices[index].logicalDevice, i, queueIndex, &tempQueueUtils.vkQueue);

				if (userDevices[index].queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					userDevices[index].graphicsQueues.push_back(tempQueueUtils);
				}
				else if (userDevices[index].queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					userDevices[index].computeQueues.push_back(tempQueueUtils);
				}
				else if (userDevices[index].queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					userDevices[index].transferQueues.push_back(tempQueueUtils);
				}
			}
		}

		if (userDevices[index].computeQueues.size() == 0)
		{
			userDevices[index].computeQueues.push_back(userDevices[index].graphicsQueues[0]);
		}

		if (userDevices[index].transferQueues.size() == 0)
		{
			userDevices[index].transferQueues.push_back(userDevices[index].graphicsQueues[0]);
		}
	}
}


void DeviceManager::chooseMainDevice()
{
	bool chooseMainDevice = false;

	for (size_t i = 0; i < userDevices.size(); i++)
    {
        if (userDevices[i].deviceFeaturesUtils.supportedProperties.properties.deviceName[0] == 'N')
		{
            mainDevice = userDevices[i];
            chooseMainDevice = true;
            return;
		}
    }

	for (size_t i = 0; i < userDevices.size(); i++)
	{
		if (userDevices[i].deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			mainDevice = userDevices[i];
			chooseMainDevice = true;
			return;
		}
	}

	if (!chooseMainDevice)
	{
		for (size_t i = 0; i < userDevices.size(); i++)
		{
			if (userDevices[i].deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER)
			{
				mainDevice = userDevices[i];
				chooseMainDevice = true;
				return;
			}
		}
	}
	
	if (!chooseMainDevice)
	{
		for (size_t i = 0; i < userDevices.size(); i++)
		{
			if (userDevices[i].deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
			{
				mainDevice = userDevices[i];
				chooseMainDevice = true;
				return;
			}
		}
	}

	if (!chooseMainDevice)
	{
		mainDevice = userDevices[0];
	}
}


bool DeviceManager::createCommandBuffers()
{
	//for (auto& device: userDevices)
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = mainDevice.graphicsQueues[0].queueFamilyIndex;
		VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &poolInfo, nullptr, &mainDevice.commandPool);
		if  (result!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics command pool!");
			//return false;
		}

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mainDevice.commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(mainDevice.logicalDevice, &allocInfo, &mainDevice.commandBuffers);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
			//return false;
		}

		return true;
	}
}
//
//
//bool DeviceManager::executeSingleTimeCommands(std::function<void(VkCommandBuffer& commandBuffer)> commandsFunction)
//{
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//	vkBeginCommandBuffer(mainDevice.commandBuffers, &beginInfo);
//
//	commandsFunction(mainDevice.commandBuffers);
//
//	vkEndCommandBuffer(mainDevice.commandBuffers);
//
//	VkCommandBufferSubmitInfo commandBufferSubmitInfos{};
//	commandBufferSubmitInfos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
//	commandBufferSubmitInfos.commandBuffer = mainDevice.commandBuffers;
//
//	VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo{};
//	waitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
//	waitSemaphoreSubmitInfo.semaphore = timelineSemaphore;
//	waitSemaphoreSubmitInfo.value = semaphoreValue++;
//	waitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
//
//	VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo{};
//	signalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
//	signalSemaphoreSubmitInfo.semaphore = timelineSemaphore;
//	signalSemaphoreSubmitInfo.value = semaphoreValue;
//	signalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
//
//	VkSubmitInfo2 submitInfo{};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
//	submitInfo.waitSemaphoreInfoCount = 1;
//	submitInfo.pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo;
//	submitInfo.signalSemaphoreInfoCount = 1;
//	submitInfo.pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo;
//	submitInfo.commandBufferInfoCount = 1;
//	submitInfo.pCommandBufferInfos = &commandBufferSubmitInfos;
//
//	VkResult result = vkQueueSubmit2(mainDevice.graphicsQueues[0].vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
//	if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to submit command buffer!");
//	}
//
//	// Wait for the timeline semaphore to reach the desired value
//	VkSemaphoreWaitInfo waitInfo{};
//	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
//	waitInfo.semaphoreCount = 1;
//	waitInfo.pSemaphores = &timelineSemaphore;
//	waitInfo.pValues = &semaphoreValue;
//
//	vkWaitSemaphores(mainDevice.logicalDevice, &waitInfo, UINT64_MAX);
//
//	return true;
//}

bool DeviceManager::executeSingleTimeCommands(std::function<void(VkCommandBuffer& commandBuffer)> commandsFunction)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mainDevice.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(mainDevice.logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	commandsFunction(commandBuffer);

	vkEndCommandBuffer(commandBuffer);

	VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
	commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	commandBufferSubmitInfo.commandBuffer = commandBuffer;

	VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo{};
	waitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	waitSemaphoreSubmitInfo.semaphore = timelineSemaphore;
	waitSemaphoreSubmitInfo.value = semaphoreValue++;
	waitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

	VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo{};
	signalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signalSemaphoreSubmitInfo.semaphore = timelineSemaphore;
	signalSemaphoreSubmitInfo.value = semaphoreValue;
	signalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.waitSemaphoreInfoCount = 1;
	submitInfo.pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo;
	submitInfo.signalSemaphoreInfoCount = 1;
	submitInfo.pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VkFence fence;
	vkCreateFence(mainDevice.logicalDevice, &fenceInfo, nullptr, &fence);

	VkResult result = vkQueueSubmit2(mainDevice.graphicsQueues[0].vkQueue, 1, &submitInfo, fence);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffer!");
	}

	// No need to wait for the fence here, just free the command buffer
	// Ensure the command buffer is not in use before freeing it
	vkWaitForFences(mainDevice.logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(mainDevice.logicalDevice, fence, nullptr);
	vkFreeCommandBuffers(mainDevice.logicalDevice, mainDevice.commandPool, 1, &commandBuffer);

	return true;
}
