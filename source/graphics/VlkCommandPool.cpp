#include "VlkCommandPool.h"

#include "VlkCommandBuffer.h"
#include "logger/Debug.h"
#include <vulkan/vulkan_core.h>

VlkCommandPool::VlkCommandPool(/* args */) {}

VlkCommandPool::~VlkCommandPool()
{
	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
}

void VlkCommandPool::Init(VkDevice device, int32 queueFamilyIndex)
{
	m_Device = device;
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.pNext = nullptr;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = queueFamilyIndex;

	VERIFY(vkCreateCommandPool(m_Device, &poolCreateInfo, nullptr, &m_CommandPool) == VK_SUCCESS,
		   "Failed to create VkCommandPool");
}

Core::GrowingArray<VlkCommandBuffer*> VlkCommandPool::CreateCommandBuffers(CommandBufferLevel bufferLevel,
																		  CommandBufferUsage usage, int32 bufferCount)
{
	Core::GrowingArray<VlkCommandBuffer*> buffers(bufferCount);

	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = m_CommandPool;
	info.commandBufferCount = bufferCount;
	info.level = (VkCommandBufferLevel)bufferLevel;

	auto vkBuffers = std::make_unique<VkCommandBuffer[]>(bufferCount);
	VERIFY(vkAllocateCommandBuffers(m_Device, &info, vkBuffers.get()) == VK_SUCCESS,
		   "Failed to create VkCommandBuffer");

	for(int32 i = 0; i < bufferCount; ++i)
	{
		buffers.Add(new VlkCommandBuffer());
		VlkCommandBuffer* buffer = buffers[i];
		buffer->Init(this, usage, bufferLevel, vkBuffers[i]);
	}

	return buffers;
}

void VlkCommandPool::DestroyBuffer(VkCommandBuffer commandBuffer)
{
	vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}
