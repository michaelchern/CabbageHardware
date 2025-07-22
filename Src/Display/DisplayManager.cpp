#include"DisplayManager.h"

#include <algorithm>

#include<Volk/volk.h>

#include<Hardware/GlobalContext.h>


//#if _WIN32 || _WIN64
//#include<vulkan/vulkan_win32.h>
//#elif __APPLE__
//#include<vulkan/vulkan_macos.h>
//#elif __linux__
//#include<vulkan/vulkan_xcb.h>
//#endif

DisplayManager::DisplayManager()
{
}


DisplayManager::~DisplayManager()
{
	//cleaarupDisplayManager();

	//vkDestroySwapchainKHR(displayDevice.logicalDevice, swapChain, nullptr);
	// Destroy the contexts
	//if (m_UpscalingContext)
	//{
	//	ffx::DestroyContext(m_UpscalingContext);
	//	m_UpscalingContext = nullptr;
	//}
}


void DisplayManager::cleaarupDisplayManager()
{
	//if (displayDevice.logicalDevice != VK_NULL_HANDLE)
	//{
	//	//vkDeviceWaitIdle(displayDevice.logicalDevice);

	//	for (size_t i = 0; i < swapChainImages.size(); i++)
	//	{
 //          displayDevice->deviceManager.deviceUtils->resourceManager.destroyImage(swapChainImages[i]);
	//	}
	//	swapChainImages.clear();

	//	if (swapChain != VK_NULL_HANDLE)
	//	{
	//		vkDestroySwapchainKHR(displayDevice.logicalDevice, swapChain, nullptr);
	//		swapChain = VK_NULL_HANDLE;
	//	}
	//	if (vkSurface != VK_NULL_HANDLE)
	//	{
 //           vkDestroySurfaceKHR(globalHardwareContext.getVulkanInstance(), vkSurface, nullptr);
	//		vkSurface = VK_NULL_HANDLE;
	//	}
		//if (frameFence != VK_NULL_HANDLE)
		//{
		//	vkDestroyFence(displayDevice.logicalDevice, frameFence, nullptr);
		//	frameFence = VK_NULL_HANDLE;
		//}
		//if (GuiFence != VK_NULL_HANDLE)
		//{
		//	vkDestroyFence(displayDevice.logicalDevice, frameFence, nullptr);
		//}
	//}
}


bool DisplayManager::initDisplayManager(void* surface)
{
	if (surface != nullptr)
	{
		//cleaarupDisplayManager();

		createVkSurface(surface);

		choosePresentDevice();

		createSwapChain();

		createFrameFence();
	}

	return true;
}


void DisplayManager::createFrameFence()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	swapchainSemaphore.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		if (vkCreateSemaphore(displayDevice->deviceManager.deviceUtils.logicalDevice, &semaphoreInfo, nullptr, &swapchainSemaphore[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}


void DisplayManager::createVkSurface(void* surface)
{
#if _WIN32 || _WIN64
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = (HWND)(surface);
	createInfo.hinstance = GetModuleHandle(NULL);

	if (vkCreateWin32SurfaceKHR(globalHardwareContext.getVulkanInstance(), &createInfo, nullptr, &vkSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

#elif __APPLE__
	VkMacOSSurfaceCreateInfoMVK createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	createInfo.pView = surface;

	if (vkCreateMacOSSurfaceMVK(deviceManager.getVulkanInstance(), &createInfo, nullptr, &vkSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

#elif __linux__

#endif
}



void DisplayManager::choosePresentDevice()
{
    for (int i = 0; i < globalHardwareContext.hardwareUtils.size(); i++)
	{
        for (int j = 0; j < globalHardwareContext.hardwareUtils[i].deviceManager.deviceUtils.queueFamilies.size(); j++)
		{
			VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(globalHardwareContext.hardwareUtils[i].deviceManager.deviceUtils.physicalDevice, j, vkSurface, &presentSupport);

			if (presentSupport)
			{ 
				displayDevice = &globalHardwareContext.hardwareUtils[i];

				if (globalHardwareContext.hardwareUtils[i].deviceManager.deviceUtils.graphicsQueues[0].queueFamilyIndex == j)
				{
                    presentQueues = globalHardwareContext.hardwareUtils[i].deviceManager.deviceUtils.graphicsQueues;
				}
                if (globalHardwareContext.hardwareUtils[i].deviceManager.deviceUtils.computeQueues[0].queueFamilyIndex == j)
				{
                    presentQueues = globalHardwareContext.hardwareUtils[i].deviceManager.deviceUtils.computeQueues;
				}
                if (globalHardwareContext.hardwareUtils[i].deviceManager.deviceUtils.transferQueues[0].queueFamilyIndex == j)
				{
                    presentQueues = globalHardwareContext.hardwareUtils[i].deviceManager.deviceUtils.transferQueues;
				}
			}
		}

		if (globalHardwareContext.mainDevice != displayDevice)
		{
			break;
		}
	}
}


void DisplayManager::createSwapChain()
{
	VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(displayDevice->deviceManager.deviceUtils.physicalDevice, vkSurface, &capabilities);

	this->displaySize = ktm::uvec2{
        std::clamp(capabilities.currentExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(capabilities.currentExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};


	uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(displayDevice->deviceManager.deviceUtils.physicalDevice, vkSurface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	if (formatCount != 0) 
	{
        vkGetPhysicalDeviceSurfaceFormatsKHR(displayDevice->deviceManager.deviceUtils.physicalDevice, vkSurface, &formatCount, formats.data());

		surfaceFormat = formats[0];
		for (const auto& availableFormat : formats)
		{
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surfaceFormat = availableFormat;
				break;
			}
		}
	}


	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(displayDevice->deviceManager.deviceUtils.physicalDevice, vkSurface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	if (presentModeCount != 0)
	{
        vkGetPhysicalDeviceSurfacePresentModesKHR(displayDevice->deviceManager.deviceUtils.physicalDevice, vkSurface, &presentModeCount, presentModes.data());
		for (const auto& availablePresentMode : presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = availablePresentMode;
				break;
			}
		}
	}


	uint32_t imageCount = (capabilities.maxImageCount > 0 && (capabilities.minImageCount + 1) > capabilities.maxImageCount) ?
		capabilities.maxImageCount : (capabilities.minImageCount + 1);


	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = vkSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = { this->displaySize.x, this->displaySize.y };
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	uint32_t queueFamilys[] = {displayDevice->deviceManager.deviceUtils.graphicsQueues[0].queueFamilyIndex, presentQueues[0].queueFamilyIndex};

	if (displayDevice->deviceManager.deviceUtils.graphicsQueues[0].queueFamilyIndex != presentQueues[0].queueFamilyIndex)
    {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilys;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = swapChain != VK_NULL_HANDLE ? swapChain : VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(displayDevice->deviceManager.deviceUtils.logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
		throw std::runtime_error("failed to create swap chain!");
	}

	std::vector<VkImage> swapChainVkImages;
    vkGetSwapchainImagesKHR(displayDevice->deviceManager.deviceUtils.logicalDevice, swapChain, &imageCount, nullptr);
	swapChainVkImages.resize(imageCount);
	swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(displayDevice->deviceManager.deviceUtils.logicalDevice, swapChain, &imageCount, swapChainVkImages.data());

	for (uint32_t i = 0; i < swapChainImages.size(); i++)
	{
		swapChainImages[i].imageHandle = swapChainVkImages[i];
		swapChainImages[i].imageSize = this->displaySize;
		swapChainImages[i].imageFormat = surfaceFormat.format;
		swapChainImages[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		swapChainImages[i].arrayLayers = 1;
		swapChainImages[i].mipLevels = 1;

        swapChainImages[i].device = &displayDevice->deviceManager;
        swapChainImages[i].resourceManager = &displayDevice->resourceManager;

		swapChainImages[i].pixelSize = 8;

        swapChainImages[i].imageView =displayDevice->resourceManager.createImageView(swapChainImages[i]);
	}
}



bool DisplayManager::displayFrame(void *displaySurface, HardwareImage displayImage)
{
    if (displaySurface != nullptr)
    {
        if (this->displaySurface != displaySurface)
        {
            //this->displaySize = displaySize;
            this->displaySurface = displaySurface;

            initDisplayManager(displaySurface);
        }

		VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(displayDevice->deviceManager.deviceUtils.physicalDevice, vkSurface, &capabilities);

        ktm::uvec2 displaySize = ktm::uvec2{
            std::clamp(capabilities.currentExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp(capabilities.currentExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};

        if (displaySize != this->displaySize)
        {
            vkDeviceWaitIdle(displayDevice->deviceManager.deviceUtils.logicalDevice);

            for (auto &image : swapChainImages)
            {
               displayDevice->resourceManager.destroyImage(image);
            }

            createSwapChain();
        }

        vkQueueWaitIdle(presentQueues[0].vkQueue);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(displayDevice->deviceManager.deviceUtils.logicalDevice, swapChain, UINT64_MAX, swapchainSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
        {
            auto start_time_ = std::chrono::high_resolution_clock::now();
            displayDevice->resourceManager.copyImageMemory(imageGlobalPool[*displayImage.imageID], swapChainImages[imageIndex]);
            std::cout << "Copy Time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time_) << std::endl;

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &swapchainSemaphore[currentFrame];

            VkSwapchainKHR swapChains[] = {swapChain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &imageIndex;

            result = vkQueuePresentKHR(presentQueues[0].vkQueue, &presentInfo);

            currentFrame = (currentFrame + 1) % swapChainImages.size();
        }

        return true;

    }
    else
    {
        return false;
    }
}
