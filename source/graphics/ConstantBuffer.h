#pragma once
#include "Core/Types.h"
#include "Core/Defines.h"
#include "Core/containers/GrowingArray.h"

#include <vulkan/vulkan_core.h>

class VlkDevice;
class VlkPhysicalDevice;
class ConstantBuffer
{
public:
	ConstantBuffer();
	void Init(VlkDevice* device);
	void Bind(VlkDevice* device, VlkPhysicalDevice* physDevice);
	void Destroy(VkDevice device);

	template <typename T>
	void RegVar(T* var);
	VkDescriptorBufferInfo GetBufferDesc();

private:
	uint32 m_BufferSize = 0;
	Core::GrowingArray<void*> m_Vars;
	VkMemoryRequirements m_MemoryRequirement;
	VkBuffer m_Buffer;
	VkDeviceMemory m_DeviceMemory;
};

template <typename T>
void ConstantBuffer::RegVar(T* var)
{
	m_Vars.Add(var);
	m_BufferSize += sizeof(T);
}
