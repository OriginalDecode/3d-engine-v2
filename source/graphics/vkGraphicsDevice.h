#pragma once

#include "GraphicsDevice.h"

#include "Core/utilities/utilities.h"
#include "Core/containers/Array.h"
#include "Core/Defines.h"
#include "VlkCommandPool.h"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class Window;

typedef struct Shader HShader;

class VlkInstance;
class VlkPhysicalDevice;
class VlkDevice;
class VlkSwapchain;

class vkGraphicsDevice
{
public:
	bool Init(const Window& window);

	void DrawFrame(float dt);

	void UpdateCamera(float dt);

	static void Create() { m_Instance = new vkGraphicsDevice; }
	static void Destroy()
	{
		delete m_Instance;
		m_Instance = nullptr;
	}
	static vkGraphicsDevice& Get() { return *m_Instance; }

	VlkInstance& GetVlkInstance() { return *m_VlkInstance; }
	VlkDevice& GetVlkDevice() { return *m_LogicalDevice; }

private:
	vkGraphicsDevice();
	~vkGraphicsDevice();
	static vkGraphicsDevice* m_Instance;

	VlkInstance* m_VlkInstance = nullptr;
	VlkPhysicalDevice* m_PhysicalDevice = nullptr;
	VlkDevice* m_LogicalDevice = nullptr;
	VlkSwapchain* m_Swapchain = nullptr;
	VlkCommandPool m_CommandPool;

	std::vector<VkFramebuffer> m_FrameBuffers;
	Core::Array<VlkCommandBuffer*, 2> m_Buffers;

	VkSemaphore m_AcquireNextImageSemaphore = nullptr;

	VkSemaphore m_DrawDone = nullptr;
	VkFence m_CommandFence = nullptr;
	VkFence m_FrameFence[2]{ nullptr, nullptr };
	uint32 m_Index = 0;

	VkRenderPass CreateRenderPass();
	VkPipeline CreateGraphicsPipeline();

	VkDescriptorSetLayout CreateDescriptorLayout(VkDescriptorSetLayoutBinding* descriptorBindings, int32 bindingCount);
	void CreateDescriptorPool();
	void CreateDescriptorSet();

	VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout* descriptorLayouts, int32 descriptorLayoutCount,
										  VkPushConstantRange* pushConstantRange, int32 pushConstantRangeCount);
	VkImageView CreateImageView(VkFormat format, VkImage image, VkImageAspectFlags aspectFlag);
	VkFramebuffer CreateFramebuffer(VkImageView* view, int32 attachmentCount, const Window& window);

	void CreateImage(uint32 width, uint32 height, VkFormat format, VkImageTiling imageTiling, VkImageUsageFlags usage,
					 VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkVertexInputBindingDescription CreateBindDesc();
	VkVertexInputAttributeDescription CreateAttrDesc(int location, int offset);

	void CreateDepthResources();

	VkSemaphore CreateVkSemaphore(VkDevice pDevice);

	// rewrite
	VlkCommandBuffer* BeginSingleTimeCommands();
	void EndSingleTimeCommands(VlkCommandBuffer* commandBuffer);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	// rewrite

	VkPipelineShaderStageCreateInfo CreateShaderStageInfo(VkShaderStageFlagBits stageFlags, VkShaderModule module,
														  const char* entryPoint);

	void CreateViewport(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth,
						VkViewport* viewport);

	void SetupScissorArea(uint32 width, uint32 height, int32 offsetX, int32 offsetY, VkRect2D* scissorArea);

	void SetupImGui();

	VkSubmitInfo SetupRenderCommands(int index);
	void PrepareRenderPass(VkRenderPassBeginInfo* pass_info, VkFramebuffer framebuffer, uint32 width, uint32 height);
};
