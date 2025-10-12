#include<Hardware/GlobalContext.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

// 全局硬件上下文实例
HardwareContext globalHardwareContext;

// 全局资源池：用于跨设备共享资源[7](@ref)
HardwareContext::HardwareContext() 
{
    prepareFeaturesChain();

    // 初始化volk库以加载Vulkan函数指针
    if (volkInitialize() != VK_SUCCESS)
    {
        throw std::runtime_error("Failed volkInitialize!");
    }

    createVkInstance(hardwareCreateInfos);

    // 让volk使用刚创建的Vulkan实例
    volkLoadInstance(vkInstance);

    uint32_t deviceCount = 0;
    // 获取系统中可用的物理设备数量
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    // 列出所有可用的物理设备
    std::vector<VkPhysicalDevice> devices(deviceCount);
    // 获取物理设备句柄列表
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    // 如果没有找到任何物理设备，抛出异常
    if (deviceCount <= 0)
    {
        throw std::runtime_error("Failed to find GPUs! Please buy a GPU!");
    }

    // 为每个物理设备创建对应的硬件工具集
    //hardwareUtils.resize(devices.size());
    for (size_t i = 0; i < devices.size(); i++)
    {
        
        hardwareUtils.push_back(std::make_shared<HardwareUtils>());
        // 初始化设备管理器（设置逻辑设备、队列等）
        hardwareUtils[i]->deviceManager.initDeviceManager(hardwareCreateInfos, vkInstance, devices[i]);
        // 初始化资源管理器（管理缓冲区、图像等资源）
        hardwareUtils[i]->resourceManager.initResourceManager(hardwareUtils[i]->deviceManager);
    }

    // 选择主用物理设备
    chooseMainDevice();

    // demo of mutilple devices
    //if (hardwareUtils.size()>1)
    //{
    //    ResourceManager::ImageHardwareWrap a = hardwareUtils[0].resourceManager.createImage(ktm::uvec2(800, 800), VK_FORMAT_R8G8B8A8_UINT, 4, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    //    ResourceManager::ImageHardwareWrap b = hardwareUtils[1].resourceManager.createImage(ktm::uvec2(800, 800), VK_FORMAT_R8G8B8A8_UINT, 4, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    //    hardwareUtils[0].resourceManager.copyImageMemory(a, b);
    //    hardwareUtils[0].resourceManager.copyImageMemory(b, a);
    //    hardwareUtils[1].resourceManager.copyImageMemory(a, b);
    //    hardwareUtils[1].resourceManager.copyImageMemory(b, a);
    //}
    // demo of mutilple devices
}

HardwareContext::~HardwareContext() 
{
    if (debugMessenger != VK_NULL_HANDLE)
     {
         auto DestroyDebugUtilsMessengerEXT = [](VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) -> VkResult {
             auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
             if (func != nullptr)
             {
                 func(instance, debugMessenger, pAllocator);
                 return VK_SUCCESS;
             }
             else
             {
                 return VK_ERROR_EXTENSION_NOT_PRESENT;
             }
         };
     }
    
    vkDestroyInstance(vkInstance, nullptr);
}

// 配置Vulkan实例和设备所需的特性、扩展链
void HardwareContext::prepareFeaturesChain()
{
    hardwareCreateInfos.requiredInstanceExtensions = [&](const VkInstance &instance, const VkPhysicalDevice &device) {
        std::set<const char *> requiredExtensions;

        requiredExtensions.insert("VK_KHR_surface");
        requiredExtensions.insert(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);

#if _WIN32 || _WIN64
        requiredExtensions.insert(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif __APPLE__
        requiredExtensions.insert(VK_MVK_MOLTENVK_EXTENSION_NAME);
        requiredExtensions.insert(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif __linux__
        requiredExtensions.insert(VK_KHR_XLIB_SURFACE_EXTENSION_NAME); // or VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#endif
        return requiredExtensions;
    };

    hardwareCreateInfos.requiredDeviceExtensions = [&](const VkInstance &instance, const VkPhysicalDevice &device) {
        std::set<const char *> requiredExtensions;
        requiredExtensions.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        requiredExtensions.insert(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

        requiredExtensions.insert(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        requiredExtensions.insert(VK_AMD_GPU_SHADER_HALF_FLOAT_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        requiredExtensions.insert(VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        requiredExtensions.insert(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

        requiredExtensions.insert(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        requiredExtensions.insert(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
#if _WIN32 || _WIN64
        requiredExtensions.insert(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
#elif __APPLE__
#elif __linux__
#endif
        return requiredExtensions;
    };

    hardwareCreateInfos.requiredDeviceFeatures = [&](const VkInstance &instance, const VkPhysicalDevice &device) {

        // Vulkan 1.0 core features
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.shaderInt16 = VK_TRUE;
        deviceFeatures.wideLines = VK_TRUE;

        // Vulkan 1.1 core features
        VkPhysicalDeviceVulkan11Features deviceFeatures11{};
        deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR;
        deviceFeatures11.multiview = VK_TRUE;

        // Vulkan 1.2 core features
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
        deviceFeatures12.descriptorBindingVariableDescriptorCount = VK_TRUE;
        deviceFeatures12.descriptorIndexing = VK_TRUE;

        deviceFeatures12.timelineSemaphore = VK_TRUE;

        // Vulkan 1.3 core features
        VkPhysicalDeviceVulkan13Features deviceFeatures13{};
        deviceFeatures13.synchronization2 = VK_TRUE;

        return (DeviceFeaturesChain() | deviceFeatures | deviceFeatures13 | deviceFeatures12 | deviceFeatures11);
    };
}


void HardwareContext::createVkInstance(const CreateCallback &initInfo)
{
    // 获取所需的实例扩展
    std::set<const char *> inputExtensions = initInfo.requiredInstanceExtensions(vkInstance, nullptr);

    std::vector<const char *> requiredExtensions(inputExtensions.begin(), inputExtensions.end());
    std::vector<const char *> requiredLayers{};

#ifdef CABBAGE_ENGINE_DEBUG
    bool enableDebugLayer = false;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto &layerProperties : availableLayers)
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

        // 过滤不支持的扩展
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

    // 应用信息和实例创建信息配置
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
        auto debugCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) -> VkBool32 {
            std::cerr << " ---------- Vulkan Validation Layer ------ \n"
                      << pCallbackData->pMessage << std::endl;
            return VK_FALSE;
        };

        debugCreateInfo.pfnUserCallback = debugCallback;

        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
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
        auto CreateDebugUtilsMessengerEXT = [](VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) -> VkResult {
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

/**
 * @brief 选择主用物理设备，优先选择独立显卡（Discrete GPU）
 * 
 * 该函数按照设备类型的优先级顺序遍历可用的硬件设备：
 * 1. 离散GPU (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) - 最高优先级，通常是独立显卡
 * 2. 其他类型 (VK_PHYSICAL_DEVICE_TYPE_OTHER) - 自定义或特殊设备
 * 3. 虚拟GPU (VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) - 虚拟化环境中的GPU
 * 4. 集成GPU (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) - 集成显卡
 * 5. CPU (VK_PHYSICAL_DEVICE_TYPE_CPU) - 最低优先级，作为备用选择
 * 
 * 如果找到匹配的设备，将其设置为 mainDevice 并立即返回。
 * 如果没有任何可用设备，则抛出运行时错误异常。
 * 
 * @throws std::runtime_error 当未找到任何可用的物理设备时抛出[1](@ref)
 */
void HardwareContext::chooseMainDevice()
{
    //for (size_t i = 0; i < hardwareUtils.size(); i++)
    //{
    //    if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceName[0] == 'N')
    //    {
    //        mainDevice = &hardwareUtils[i];
    //        return;
    //    }
    //}

    // 优先选择离散GPU（独立显卡），因其通常提供最佳图形性能[6](@ref)
    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    // 其次选择OTHER类型设备（可能是特殊计算设备或未分类的高性能设备）
    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    // 再次选择虚拟GPU（适用于虚拟化环境）
    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    // 然后选择集成GPU（集成显卡，性能一般但功耗较低）
    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    // 如果存在至少一个设备，则选择第一个作为默认设备[8](@ref)
    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    // 如果存在至少一个设备，则选择第一个作为默认设备[8](@ref)
    if (hardwareUtils.size() > 0)
    {
        mainDevice = hardwareUtils[0];
    }
    else
    {
        // 未找到任何可用的物理设备，抛出异常[1](@ref)
        throw std::runtime_error("Failed to find GPUs! Please buy a GPU!");
    }
}
