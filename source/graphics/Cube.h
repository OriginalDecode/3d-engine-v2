#pragma once
#include "Core/Types.h"
#include "Core/Defines.h"

#include "Core/math/Matrix44.h"

class VlkDevice;
class VlkPhysicalDevice;

DEFINE_HANDLE(VkBuffer);
DEFINE_HANDLE(VkDeviceMemory);
DEFINE_HANDLE(VkDevice);
DEFINE_HANDLE(VkCommandBuffer);
DEFINE_HANDLE(VkPipelineLayout);


// Vertex Description
class VlkCommandBuffer;
struct Vertex
{
	Core::Vector4f position;
	Core::Vector4f color;
	Core::Vector4f normal;
};

struct VertexBuffer
{
	VkBuffer m_Buffer;
	VkDeviceMemory m_Memory;
	int32 m_VertexCount = 0;
	int32 m_Stride = 0;
	int32 m_Offset = 0;
};

class Cube
{
public:
	Cube() = default;
	~Cube() = default;
	void Update(float /*dt*/);

	void Draw(VlkCommandBuffer* commandBuffer, VkPipelineLayout pipelineLayout);

	void Destroy(VkDevice device);

	void Init(VlkDevice* device, VlkPhysicalDevice* physicalDevice);

	void SetPosition(const Core::Vector4f& position);

private:
	VertexBuffer m_VertexBuffer;
	Core::Matrix44f m_Orientation;
};
