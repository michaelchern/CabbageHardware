#pragma once


#include"CabbageFramework/CabbageFoundation/HardwareManager/DeviceManager.h"
#include"CabbageFramework/CabbageFoundation/HardwareManager/DisplayManager.h"
#include"CabbageFramework/CabbageFoundation/HardwareManager/ResourceManager.h"




struct HardwareContext
{
    HardwareContext()
	{
		DeviceManager::CreateCallback hardwareCreateInfos{};

		hardwareCreateInfos.requiredInstanceExtensions = [&](const VkInstance& instance, const VkPhysicalDevice& device)
			{
				std::set<const char *> requiredExtensions = DisplayManager::checkInstanceExtensionRequirements(instance, device);
				requiredExtensions.insert(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#if __APPLE__
				requiredExtensions.insert(VK_MVK_MOLTENVK_EXTENSION_NAME);
#endif
				return requiredExtensions;
			};

		hardwareCreateInfos.requiredDeviceExtensions = [&](const VkInstance& instance, const VkPhysicalDevice& device)
			{
				std::set<const char *> requiredExtensions = DisplayManager::checkDeviceExtensionRequirements(instance, device);
				requiredExtensions.insert(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
				requiredExtensions.insert(VK_KHR_MULTIVIEW_EXTENSION_NAME);
				requiredExtensions.insert(VK_AMD_GPU_SHADER_HALF_FLOAT_EXTENSION_NAME);
				requiredExtensions.insert(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
				requiredExtensions.insert(VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME);
				requiredExtensions.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

				requiredExtensions.insert(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
				requiredExtensions.insert(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

				return requiredExtensions;
			};

		hardwareCreateInfos.requiredDeviceFeatures = [&](const VkInstance& instance, const VkPhysicalDevice& device)
			{
				VkPhysicalDeviceFeatures deviceFeatures{};
				deviceFeatures.samplerAnisotropy = VK_TRUE;
				deviceFeatures.shaderInt16 = VK_TRUE;
				deviceFeatures.wideLines = VK_TRUE;

				VkPhysicalDeviceVulkan13Features deviceFeatures13{};
				deviceFeatures13.synchronization2 = VK_TRUE;

				VkPhysicalDeviceVulkan12Features deviceFeatures12{};
				deviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
				deviceFeatures12.bufferDeviceAddress = VK_TRUE;
				deviceFeatures12.shaderFloat16 = VK_TRUE;
				deviceFeatures12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
				deviceFeatures12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
				deviceFeatures12.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
				deviceFeatures12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
				deviceFeatures12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
				deviceFeatures12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
				deviceFeatures12.descriptorBindingPartiallyBound = VK_TRUE;
				deviceFeatures12.runtimeDescriptorArray = VK_TRUE;
				deviceFeatures12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
				deviceFeatures12.timelineSemaphore = VK_TRUE;

				VkPhysicalDeviceVulkan11Features deviceFeatures11{};
				deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR;
				deviceFeatures11.multiview = VK_TRUE;

				return (DeviceManager::DeviceFeaturesChain() | deviceFeatures | deviceFeatures13 | deviceFeatures12 | deviceFeatures11);
			};

		deviceManager.initDeviceManager(hardwareCreateInfos);
		resourceManager.initResourceManager();
	}

	~HardwareContext() = default;

	DeviceManager deviceManager;

	DeviceManager::DeviceUtils mainDevice;
    std::vector<DeviceManager::DeviceUtils> userDevices;

	ResourceManager resourceManager;
    std::vector<DisplayManager> displayManagers;

};

extern HardwareContext globalHardwareContext;