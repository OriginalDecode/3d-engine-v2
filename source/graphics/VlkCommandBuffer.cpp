#include "VlkCommandBuffer.h"

#include "vkGraphicsDevice.h"
#include "VlkCommandPool.h"

#include "logger/Debug.h"

#include <vulkan/vulkan_core.h>

VlkCommandBuffer::VlkCommandBuffer() {}

VlkCommandBuffer::~VlkCommandBuffer()
{
	m_Owner->DestroyBuffer(m_Buffer);
	m_Buffer = nullptr;
}

void VlkCommandBuffer::Init(VlkCommandPool* pool, CommandBufferUsage usage, CommandBufferLevel bufferLevel,
							VkCommandBuffer buffer)
{
	m_Owner = pool;
	m_BeginInfo = VkCommandBufferBeginInfo();
	m_BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	m_BeginInfo.flags = (VkCommandBufferUsageFlags)usage;

	m_UsageType = usage;
	m_BufferLevel = bufferLevel;
	m_Buffer = buffer;
}

void VlkCommandBuffer::Begin()
{
	VERIFY(vkBeginCommandBuffer(m_Buffer, &m_BeginInfo) == VK_SUCCESS, "vkBeginCommandBuffer failed!");
}

VkSubmitInfo VlkCommandBuffer::End()
{
	VERIFY(vkEndCommandBuffer(m_Buffer) == VK_SUCCESS, "vkEndCommandBuffer failed!");

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_Buffer;
	return submitInfo;
}

void VlkCommandBuffer::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(m_Buffer, src, dst, 1, &copyRegion);
}

void VlkCommandBuffer::SetPipelineBarriers(PipelineBarrierSetupInfo info)
{
	vkCmdPipelineBarrier(m_Buffer, info.srcStageMask, info.dstStageMask, info.dependencyFlags, info.memoryBarrierCount,
						 info.pMemoryBarriers, info.bufferMemoryBarrierCount, info.pBufferMemoryBarriers,
						 info.imageMemoryBarrierCount, info.pImageMemoryBarriers);
}

void VlkCommandBuffer::BindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
	vkCmdBindPipeline(m_Buffer, pipelineBindPoint, pipeline);
}

void VlkCommandBuffer::BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
										  uint32_t firstSet, uint32_t descriptorSetCount,
										  const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
										  const uint32_t* pDynamicOffsets)
{
	vkCmdBindDescriptorSets(m_Buffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
							dynamicOffsetCount, pDynamicOffsets);
}

void VlkCommandBuffer::BeginRenderPass(const VkRenderPassBeginInfo& beginInfo, VkSubpassContents contents)
{
	vkCmdBeginRenderPass(m_Buffer, &beginInfo, contents);
}

void VlkCommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(m_Buffer);
}

void VlkCommandBuffer::DrawDirect(uint32 vertexCount, uint32 instanceCount, uint32 vertexStart, uint32 instanceStart)
{
	vkCmdDraw(m_Buffer, vertexCount, instanceCount, vertexStart, instanceStart);
}

void VlkCommandBuffer::PushConstants(VkPipelineLayout pipelineLayout, uint32 stageBit, uint32 offset, uint32 size, const void* pValues)
{
	vkCmdPushConstants(m_Buffer, pipelineLayout, stageBit, offset, size, pValues);
}

void VlkCommandBuffer::BindVertexBuffers(uint32 startBindingPos, uint32 nofBindings, const VkBuffer* buffer, VkDeviceSize* offset)
{
	vkCmdBindVertexBuffers(m_Buffer, startBindingPos, nofBindings, buffer, offset);
}