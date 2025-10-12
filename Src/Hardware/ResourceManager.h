#pragma once

#include <unordered_map>

#include"DeviceManager.h"

#include<vk_mem_alloc.h>

#include <ktm/ktm.h>

/**
 * @brief Vulkan资源管理核心类，集成VMA（Vulkan Memory Allocator）用于高效内存管理
 * 
 * 功能包括：
 * - 图像和缓冲区的创建与销毁
 * - 绑定描述符集管理（支持bindless设计）
 * - 跨平台外部内存句柄导入/导出（适用于多GPU或进程间共享）
 * - 资源拷贝和布局转换
 * 
 * @warning 该类非线程安全，多线程环境下需外部同步机制保障
 */
struct ResourceManager
{
	/**
     * @brief 跨平台外部内存句柄，用于进程间或API间资源共享
     * 
     * @details 在Windows平台使用HANDLE，类Unix平台使用文件描述符（fd）
     * 适用于Vulkan与其他图形API（如DirectX、OpenGL）间的资源交互[7](@ref)
     */
    struct ExternalMemoryHandle
    {
#if _WIN32 || _WIN64
        HANDLE handle = nullptr;
#else
        int fd = -1;
#endif
    };

	/**
     * @brief 硬件缓冲区封装类，管理VkBuffer及其内存分配
     * 
     * @property bufferHandle Vulkan缓冲区对象句柄
     * @property bufferAlloc VMA分配对象，用于内存管理
     * @property bufferUsage 缓冲区用途标志（如传输、存储等）
     * @property stageMask 管线阶段掩码，用于同步
     * @property accessMask 访问权限掩码，用于屏障控制
     */
	struct BufferHardwareWrap
	{
        VkPipelineStageFlags stageMask; ///< 缓冲区使用的管线阶段
        VkAccessFlags accessMask; ///< 缓冲区访问权限

		VkBuffer bufferHandle = VK_NULL_HANDLE;
		VmaAllocation bufferAlloc = VK_NULL_HANDLE;
		VmaAllocationInfo bufferAllocInfo = {}; ///< 内存分配详细信息
		VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM; ///< 缓冲区使用标志

        DeviceManager *device; ///< 关联的设备管理器
        ResourceManager *resourceManager; ///< 父资源管理器
	};

	/**
     * @brief 硬件图像封装类，管理VkImage及其视图、内存分配
     * 
     * @property imageHandle Vulkan图像对象句柄
     * @property imageView 图像视图，用于着色器访问
     * @property imageFormat 图像像素格式（如VK_FORMAT_R8G8B8A8_UNORM）
     * @property imageSize 图像尺寸（宽、高）
     * @property mipLevels 多级渐远纹理层级数
     * @property arrayLayers 图像数组层数（用于纹理数组）
     * @property imageLayout 图像当前布局（影响渲染和拷贝操作）
     */
	struct ImageHardwareWrap
	{
        VkPipelineStageFlags stageMask; ///< 图像使用的管线阶段
        VkAccessFlags accessMask; ///< 图像访问权限

		uint32_t pixelSize = 0; ///< 单个像素字节大小
		ktm::uvec2 imageSize = ktm::uvec2(0, 0); ///< 图像尺寸（宽度、高度）
		VkFormat imageFormat = VK_FORMAT_MAX_ENUM; ///< 图像像素格式
		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM; ///< 图像使用标志
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_NONE; ///< 图像切面掩码（如颜色、深度）
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED; ///< 图像当前布局

		int arrayLayers = 1; ///< 图像数组层数
		int mipLevels = 1; ///< 多级渐远层级数

		VkClearValue clearValue = {}; ///< 清除值（用于渲染附件）

		VmaAllocation imageAlloc = VK_NULL_HANDLE; ///< VMA内存分配对象
		VmaAllocationInfo imageAllocInfo = {}; ///< 内存分配详细信息

		VkImage imageHandle = VK_NULL_HANDLE; ///< Vulkan图像句柄
		VkImageView imageView = VK_NULL_HANDLE; ///< 图像视图句柄

		DeviceManager *device; ///< 关联的设备管理器
        ResourceManager *resourceManager; ///< 父资源管理器
	};


	/**
     * @brief Bindless描述符集管理结构，支持动态资源绑定
     * 
     * @details 包含描述符池、布局和集合，用于实现Bindless渲染技术，
     *          允许着色器动态索引资源而无需频繁更新描述符集[6](@ref)
     */
	struct
    {
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE; ///< 描述符池
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE; ///< 描述符集布局
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE; ///< 描述符集
    } bindlessDescriptors[4]; ///< 四类绑定点：Uniform、纹理、存储缓冲区、存储图像
    

	/**
     * @brief 初始化资源管理器
     * @param device 设备管理器引用，用于创建Vulkan对象
     */
	ResourceManager();

	/**
     * @brief 销毁图像资源，释放相关Vulkan对象和内存
     * @param image 待销毁的图像对象引用
     */
    void initResourceManager(DeviceManager &device);

	/**
     * @brief 销毁图像资源，释放相关Vulkan对象和内存
     * @param image 待销毁的图像对象引用
     */
	void destroyImage(ImageHardwareWrap& image);

	/**
     * @brief 为图像创建视图，用于着色器采样或访问
     * @param image 目标图像对象
     * @return 创建的图像视图句柄
     */
	VkImageView createImageView(ImageHardwareWrap& image);

	/**
     * @brief 创建图像资源
     * @param imageSize 图像尺寸（宽、高）
     * @param imageFormat 像素格式
     * @param pixelSize 单个像素字节大小
     * @param imageUsage 图像使用标志（默认包含传输和采样）
     * @param arrayLayers 图像数组层数（默认1）
     * @param mipLevels 多级渐远层级数（默认1）
     * @return 创建的图像封装对象
     */
    ImageHardwareWrap createImage(ktm::uvec2 imageSize, VkFormat imageFormat, uint32_t pixelSize,
		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		int arrayLayers = 1, int mipLevels = 1);


	/**
     * @brief 销毁缓冲区资源，释放相关Vulkan对象和内存
     * @param buffer 待销毁的缓冲区对象引用
     */
	void destroyBuffer(BufferHardwareWrap& buffer);
	BufferHardwareWrap createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);


	/**
     * @brief 将图像资源绑定到描述符集，返回描述符索引（Bindless技术）
     * @param image 目标图像对象
     * @return 描述符索引，用于着色器访问
     */
	uint32_t storeDescriptor(ImageHardwareWrap image);

	/**
     * @brief 将缓冲区资源绑定到描述符集，返回描述符索引
     * @param buffer 目标缓冲区对象
     * @return 描述符索引，用于着色器访问
     */
	uint32_t storeDescriptor(BufferHardwareWrap buffer);
	//uint32_t storeDescriptor(VkAccelerationStructureKHR m_tlas);

	/**
     * @brief 拷贝图像内存内容（支持暂存缓冲区优化）
     * @param source 源图像
     * @param destination 目标图像
     * @param srcStaging 源暂存缓冲区（可选）
     * @param dstStaging 目标暂存缓冲区（可选）
     * @return 拷贝是否成功
     */
	bool copyImageMemory(ImageHardwareWrap &source, ImageHardwareWrap &destination, BufferHardwareWrap *srcStaging = nullptr, BufferHardwareWrap *dstStaging = nullptr);

	/**
     * @brief 从缓冲区拷贝数据到图像
     * @param buffer 源缓冲区
     * @param image 目标图像
     * @param width 图像宽度
     * @param height 图像高度
     */
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	/**
     * @brief 缓冲区间数据拷贝
     * @param srcBuffer 源缓冲区
     * @param dstBuffer 目标缓冲区
     * @param size 拷贝数据大小
     */
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	/**
     * @brief 导出图像内存句柄，用于跨进程或跨API共享
     * @param sourceImage 源图像对象
     * @return 外部内存句柄[7](@ref)
     */
    ExternalMemoryHandle exportImageMemory(ImageHardwareWrap &sourceImage);

	/**
     * @brief 导入外部内存句柄创建图像
     * @param memHandle 外部内存句柄
     * @param sourceImage 参考图像（用于获取格式和尺寸）
     * @return 新创建的图像对象
     */
    ImageHardwareWrap importImageMemory(const ExternalMemoryHandle &memHandle, const ImageHardwareWrap &sourceImage);

	/**
     * @brief 查找支持外部内存的内存类型索引
     * @param typeFilter 内存类型过滤器
     * @param properties 内存属性要求
     * @return 匹配的内存类型索引
     */
	uint32_t findExternalMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	//void transitionImageLayoutUnblocked(const VkCommandBuffer& commandBuffer,ImageHardwareWrap& image, VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

	//void transitionImageLayout(ImageHardwareWrap& image, VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

	/**
     * @brief 创建着色器模块
     * @param code SPIR-V字节码数据
     * @return 着色器模块句柄
     */
	VkShaderModule createShaderModule(const std::vector<unsigned int>& code);

	/**
     * @brief 获取主机共享内存大小（用于CPU-GPU共享资源）
     * @return 主机共享内存大小（字节）
     */
	uint64_t getHostSharedMemorySize()
	{
		return hostSharedMemorySize;
	}

	/**
     * @brief 获取设备专用内存大小（用于GPU独占资源）
     * @return 设备内存大小（字节）
     */
	uint64_t getDeviceMemorySize()
	{
		return deviceMemorySize;
	}

private:
    /// @brief 创建纹理采样器，用于着色器采样操作
	void createTextureSampler();

	/// @brief 初始化VMA（Vulkan Memory Allocator）实例
	void CreateVmaAllocator();

	/// @brief 创建Bindless描述符集布局和池
	void createBindlessDescriptorSet();

	///< VMA分配器实例，用于高效内存管理[4](@ref)
	VmaAllocator g_hAllocator;

	VkSampler textureSampler; ///< 默认纹理采样器

	const uint32_t UniformBinding = 0; ///< Uniform缓冲区绑定点
	const uint32_t TextureBinding = 1; ///< 纹理绑定点
	const uint32_t StorageBufferBinding = 2; ///< 存储缓冲区绑定点
	const uint32_t StorageImageBinding = 3;  ///< 存储图像绑定点

	std::unordered_map<VkBuffer, int> UniformBindingList; ///< Uniform缓冲区绑定映射
	std::unordered_map<VkImageView, int> TextureBindingList; ///< 纹理绑定映射
	std::unordered_map<VkBuffer, int> StorageBufferBindingList; ///< 存储缓冲区绑定映射
	std::unordered_map<VkImageView, int> StorageImageBindingList; ///< 存储图像绑定映射

	std::mutex bindlessDescriptorMutex; ///< 描述符绑定操作的互斥锁（保证线程安全）

	uint32_t UniformBindingIndex = 0; ///< Uniform缓冲区当前绑定索引
    uint32_t TextureBindingIndex = 0; ///< 纹理当前绑定索引
    uint32_t StorageBufferBindingIndex = 0; ///< 存储缓冲区当前绑定索引
    uint32_t StorageImageBindingIndex = 0; ///< 存储图像当前绑定索引

	uint64_t deviceMemorySize = 0; ///< 设备专用内存总量
	uint64_t hostSharedMemorySize = 0; ///< 主机共享内存总量
	uint64_t mutiInstanceMemorySize = 0; ///< 多实例共享内存总量

	DeviceManager *device; ///< 关联的设备管理器实例
};