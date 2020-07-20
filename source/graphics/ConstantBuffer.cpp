#include "ConstantBuffer.h"

#include "VlkDevice.h"
#include "VlkPhysicalDevice.h"

#include <vulkan/vulkan_core.h>

ConstantBuffer::ConstantBuffer()
{
	m_MemoryRequirement = {};
}

void ConstantBuffer::Init(VlkDevice* device)
{
	if(m_Vars.Size() <= 0)
	{
		ASSERT(false, "You have to Register some variables before initializing the constant buffer!");
		return;
	}

	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = m_BufferSize;
	createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	auto [buffer, memReq] = device->CreateBuffer(createInfo);
	m_Buffer = buffer;
	m_MemoryRequirement = memReq;
}

void ConstantBuffer::Bind(VlkDevice* device, VlkPhysicalDevice* physDevice)
{
	VkDeviceMemory memory = device->BindBuffer(m_Buffer, m_MemoryRequirement, physDevice);
	int8* data = (int8*)device->MapMemory(memory, 0, m_BufferSize, 0);
	memcpy(data, &m_Vars[0], m_BufferSize);

	device->UnmapMemory(memory);
}

void ConstantBuffer::Destroy(VkDevice device)
{
	vkDestroyBuffer(device, m_Buffer, nullptr);
}

VkDescriptorBufferInfo ConstantBuffer::GetBufferDesc()
{
	return VkDescriptorBufferInfo{ .buffer = m_Buffer, .offset = 0, .range = m_BufferSize };
}
