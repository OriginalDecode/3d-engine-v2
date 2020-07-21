#pragma once
#include "Core/Defines.h"
#include "Core/Types.h"

#include "VlkCommandBuffer.h"

#include "Core/containers/GrowingArray.h"

#ifndef VkCommandPool
DEFINE_HANDLE(VkCommandPool);
#endif

#ifndef VkDevice
DEFINE_HANDLE(VkDevice);
#endif

class VlkCommandBuffer;
class VlkCommandPool
{
public:
	VlkCommandPool();
	~VlkCommandPool();

	void Init(VkDevice device, int32 queueFamilyIndex);

	Core::GrowingArray<VlkCommandBuffer*> CreateCommandBuffers(CommandBufferLevel bufferLevel, CommandBufferUsage usage, int32 bufferCount);
	void DestroyBuffer(VkCommandBuffer commandBuffer);

private:
	VkCommandPool m_CommandPool = nullptr;
	VkDevice m_Device = nullptr;
};
