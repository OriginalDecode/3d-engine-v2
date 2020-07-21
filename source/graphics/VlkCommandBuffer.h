#pragma once

#include "Core/Defines.h"
#include "Core/Types.h"

#include <vulkan/vulkan_core.h>

DEFINE_HANDLE(VkDevice);
DEFINE_HANDLE(VkCommandBuffer);

class VlkCommandPool;

enum class CommandBufferLevel
{
	E_PRIMARY,
	E_SECONDARY
};

enum class CommandBufferUsage
{
	E_NONE = 0,
	E_ONE_TIME = BIT(0),
	E_RENDER_PASS_CONTINUE = BIT(1),
	E_SIMULTANEOUS = BIT(2),

};

struct PipelineBarrierSetupInfo
{
	VkPipelineStageFlags srcStageMask = 0;
	VkPipelineStageFlags dstStageMask = 0;
	VkDependencyFlags dependencyFlags = 0;
	uint32 memoryBarrierCount = 0;
	const VkMemoryBarrier* pMemoryBarriers = nullptr;
	uint32 bufferMemoryBarrierCount = 0;
	const VkBufferMemoryBarrier* pBufferMemoryBarriers = nullptr;
	uint32 imageMemoryBarrierCount = 0;
	const VkImageMemoryBarrier* pImageMemoryBarriers = nullptr;
};

class VlkCommandBuffer
{
public:
	VlkCommandBuffer();
	~VlkCommandBuffer();

	void Init(VlkCommandPool* pool, CommandBufferUsage usage, CommandBufferLevel bufferLevel, VkCommandBuffer buffer);

	void Begin();
	VkSubmitInfo End();

	void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

	void SetPipelineBarriers(PipelineBarrierSetupInfo info);
	void BindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
	void BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet,
							uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets,
							uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
	void BeginRenderPass(const VkRenderPassBeginInfo& beginInfo, VkSubpassContents contents);
	void EndRenderPass();

	void PushConstants(VkPipelineLayout pipelineLayout, uint32 stageBit, uint32 offset, uint32 size, const void* pValues);
	void BindVertexBuffers(uint32 startBindingPos, uint32 nofBindings, const VkBuffer* buffer, VkDeviceSize* offset);
	void DrawDirect(uint32 vertexCount, uint32 instanceCount, uint32 vertexStart, uint32 instanceStart);
	// void DrawDirect(uint32 startBindingPos, uint32 nofBindings, VkBuffer buffer, VkDeviceSize* offset);

private:
	VlkCommandPool* m_Owner = nullptr;
	VkCommandBuffer m_Buffer = nullptr;
	CommandBufferLevel m_BufferLevel = CommandBufferLevel::E_PRIMARY;
	CommandBufferUsage m_UsageType = CommandBufferUsage::E_NONE;
	VkCommandBufferBeginInfo m_BeginInfo;
};
