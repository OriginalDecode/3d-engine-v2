#include "Cube.h"

#include "VlkDevice.h"
#include "VlkPhysicalDevice.h"
#include "VlkCommandBuffer.h"

#include "Core/File.h"
#include "Logger/Debug.h"

#include <vulkan/vulkan_core.h>
#include <memory>

void Cube::Init(VlkDevice* device, VlkPhysicalDevice* physicalDevice)
{
	Core::File loader("cube.mdl", Core::File::READ_FILE);

	m_Orientation = Core::Matrix44f::Identity();
	m_VertexBuffer.m_Stride = sizeof(Vertex);
	m_VertexBuffer.m_VertexCount = (loader.GetSize() / sizeof(Vertex));
	m_VertexBuffer.m_Offset = 0;

	const int32 dataSize = m_VertexBuffer.m_Stride * m_VertexBuffer.m_VertexCount;

	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = dataSize;
	createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	auto [buffer, memReq] = device->CreateBuffer(createInfo);
	m_VertexBuffer.m_Buffer = buffer;

	VkDeviceMemory vkmem = device->BindBuffer(m_VertexBuffer.m_Buffer, memReq, physicalDevice);
	int8* mem = (int8*)device->MapMemory(vkmem, 0, dataSize, 0);
	memcpy(mem, loader.GetBuffer(), loader.GetSize());
	device->UnmapMemory(vkmem);

	m_VertexBuffer.m_Memory = vkmem;
}

void Cube::Destroy(VkDevice device)
{
	vkDestroyBuffer(device, m_VertexBuffer.m_Buffer, nullptr);
}

void Cube::Update(float /*dt*/) {}

void Cube::Draw(VlkCommandBuffer* commandBuffer, VkPipelineLayout pipelineLayout)
{
	int8* data = new int8[sizeof(Core::Matrix44f)];
	memset(data, 0, sizeof(Core::Matrix44f));
	memcpy(data, &m_Orientation[0], sizeof(Core::Matrix44f));
	commandBuffer->PushConstants(pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Core::Matrix44f), data);
	VkDeviceSize offset = 0;
	commandBuffer->BindVertexBuffers(0, 1, &m_VertexBuffer.m_Buffer, &offset);
	// This executes secondary commandBuffers inside of the primary commandBuffer
	// vkCmdExecuteCommands(commandBuffer, 0, nullptr);
	commandBuffer->DrawDirect(m_VertexBuffer.m_VertexCount, 1, 0 , 0);

	delete[] data;
	data = nullptr;
}

void Cube::SetPosition(const Core::Vector4f& position)
{
	m_Orientation.SetPosition(position);
}
