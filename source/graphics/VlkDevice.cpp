#include "VlkDevice.h"
#include "VlkPhysicalDevice.h"
#include "VlkCommandBuffer.h"
#include "VlkCommandPool.h"

#include "logger/Debug.h"

#include <Windows.h>
#include <vulkan/vulkan_core.h>
#include <cassert>

const char* debugLayers[] = { "VK_LAYER_LUNARG_standard_validation" };
const char* deviceExt[] = { "VK_KHR_swapchain" };

VlkDevice::~VlkDevice()
{
	vkDestroyDevice(m_Device, nullptr);
}

VkSwapchainKHR VlkDevice::CreateSwapchain(const VkSwapchainCreateInfoKHR& createInfo) const
{
	VkSwapchainKHR swapchain = nullptr;
	VkResult result = vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &swapchain);
	assert(result == VK_SUCCESS && "Failed to create swapchain");
	return swapchain;
}

void VlkDevice::DestroySwapchain(VkSwapchainKHR pSwapchain)
{
	vkDestroySwapchainKHR(m_Device, pSwapchain, nullptr);
}

void VlkDevice::GetSwapchainImages(VkSwapchainKHR* pSwapchain, std::vector<VkImage>* scImages)
{
	uint32 imageCount = 0;
	VkResult result = vkGetSwapchainImagesKHR(m_Device, *pSwapchain, &imageCount, nullptr);
	assert(result == VK_SUCCESS && "failed to get imagecount");
	scImages->resize(imageCount);
	result = vkGetSwapchainImagesKHR(m_Device, *pSwapchain, &imageCount, scImages->data());
	assert(result == VK_SUCCESS && "failed to get swapchainimages");
}

VkDeviceMemory VlkDevice::AllocateMemory(const VkMemoryRequirements& requirements, VlkPhysicalDevice* physDevice)
{
	VkDeviceMemory memory;
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = requirements.size;
	allocInfo.memoryTypeIndex = physDevice->FindMemoryType(
		requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if(vkAllocateMemory(m_Device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
		ASSERT(false, "Failed to allocate memory on GPU!");

	return memory;
}

void VlkDevice::FreeMemory(VkDeviceMemory memory)
{
	vkFreeMemory(m_Device, memory, nullptr /*callback*/);
}

std::tuple<VkBuffer, VkMemoryRequirements> VlkDevice::CreateBuffer(const VkBufferCreateInfo& createInfo)
{
	VkBuffer buffer = nullptr;
	if(vkCreateBuffer(m_Device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
		ASSERT(false, "Failed to create vertex buffer!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

	return { buffer, memRequirements };
}

VkDeviceMemory VlkDevice::BindBuffer(VkBuffer buffer, const VkMemoryRequirements& requirements,
									 VlkPhysicalDevice* physDevice)
{
	VkDeviceMemory memory = AllocateMemory(requirements, physDevice);

	if(vkBindBufferMemory(m_Device, buffer, memory, 0) != VK_SUCCESS)
	{
		ASSERT(false, "Failed to bind buffer memory!");
		return nullptr;
	}

	return memory;
}

void VlkDevice::UnmapMemory(VkDeviceMemory memory)
{
	vkUnmapMemory(m_Device, memory);
}

void VlkDevice::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size, VlkCommandPool* commandPool)
{
	auto buffer = commandPool->CreateCommandBuffers(CommandBufferLevel::E_PRIMARY, CommandBufferUsage::E_ONE_TIME, 1);

	buffer[0]->Begin();
	buffer[0]->CopyBuffer(src, dst, size);
	VkSubmitInfo submitInfo = buffer[0]->End();

	vkQueueSubmit(m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_Queue);
}

VkShaderModule VlkDevice::CreateShaderModule(const char* data, uint32 dataLength)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = dataLength;
	createInfo.pCode = (const uint32_t*)data;

	VkShaderModule shaderModule = nullptr;
	if(vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		ASSERT(false, "Failed to create VkShaderModule!");
		return nullptr;
	}

	return shaderModule;
}

void VlkDevice::DestroyShaderModule(VkShaderModule* module)
{
	vkDestroyShaderModule(m_Device, *module, nullptr);
}

void* VlkDevice::MapMemory(VkDeviceMemory memory, uint32 offset, uint32 size, uint32 flags)
{
	void* data = nullptr;
	VERIFY(vkMapMemory(m_Device, memory, offset, size, 0, &data) == VK_SUCCESS, "Failed to map vulkan memory!");
	return data;
}

void VlkDevice::Init(VlkPhysicalDevice* physicalDevice)
{
	// queue create info
	const float queue_priorities[] = { 1.f };
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = physicalDevice->GetQueueFamilyIndex();
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queue_priorities;

	// Physical device features
	VkPhysicalDeviceFeatures enabled_features = {};
	enabled_features.shaderClipDistance = true;

	// device create info
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
#ifdef _DEBUG
	createInfo.enabledLayerCount = ARRSIZE(debugLayers);
	createInfo.ppEnabledLayerNames = debugLayers;
#endif
	createInfo.enabledExtensionCount = ARRSIZE(deviceExt);
	createInfo.ppEnabledExtensionNames = deviceExt;
	createInfo.pEnabledFeatures = &enabled_features;

	m_Device = physicalDevice->CreateDevice(createInfo);

	vkGetDeviceQueue(m_Device, physicalDevice->GetQueueFamilyIndex(), 0, &m_Queue);
}
