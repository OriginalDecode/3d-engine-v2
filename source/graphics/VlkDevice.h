#pragma once
#include "Core/Defines.h"
#include "Core/Types.h"

#include <vulkan/vulkan_core.h>
#include <vector>

DEFINE_HANDLE(VkDevice);
DEFINE_HANDLE(VkQueue);
DEFINE_HANDLE(VkSwapchainKHR);

struct VkSwapchainCreateInfoKHR;

class VlkPhysicalDevice;
class VlkCommandPool;
class ConstantBuffer;
class VlkDevice
{
public:
	VlkDevice() = default;
	~VlkDevice();

	void Init(VlkPhysicalDevice* physicalDevice);

	VkDevice GetDevice() const { return m_Device; }
	VkQueue GetQueue() const { return m_Queue; }

	VkSwapchainKHR CreateSwapchain(const VkSwapchainCreateInfoKHR& createInfo) const;
	void DestroySwapchain(VkSwapchainKHR pSwapchain);

	void GetSwapchainImages(VkSwapchainKHR* pSwapchain, std::vector<VkImage>* scImages);

	VkDeviceMemory AllocateMemory(const VkMemoryRequirements& requirements, VlkPhysicalDevice* physDevice);
	void FreeMemory(VkDeviceMemory memory);
	std::tuple<VkBuffer, VkMemoryRequirements> CreateBuffer(const VkBufferCreateInfo& createInfo);
	VkDeviceMemory BindBuffer(VkBuffer buffer, const VkMemoryRequirements& requirements, VlkPhysicalDevice* physDevice);

	void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size, VlkCommandPool* commandPool);

	VkShaderModule CreateShaderModule(const char* data, uint32 dataLength);
	void DestroyShaderModule(VkShaderModule* module);

	void* MapMemory(VkDeviceMemory memory, uint32 offset, uint32 size, uint32 flags);

	void UnmapMemory(VkDeviceMemory memory);

private:
	VkDevice m_Device = nullptr;
	VkQueue m_Queue = nullptr;
};
