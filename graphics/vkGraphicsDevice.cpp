#include "vkGraphicsDevice.h"

#include "Camera.h"

#include "VlkInstance.h"
#include "VlkPhysicalDevice.h"
#include "VlkDevice.h"
#include "VlkSwapchain.h"

#include "Utilities.h"
#include "Window.h"

#include "Core/File.h"
#include "Core/math/Matrix44.h"
#include "Core/utilities/Randomizer.h"
#include "Input/InputManager.h"
#include "input/InputDeviceMouse_Win32.h"
#include "input/InputDeviceKeyboard_Win32.h"

#include "logger/Debug.h"

#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

VkClearColorValue _clearColor = { 0.f, 0.f, 0.f, 0.f };

VkRenderPass _renderPass = nullptr;

VkPipeline _pipeline = nullptr;
VkPipelineLayout _pipelineLayout = nullptr;
VkViewport _Viewport = {};
VkRect2D _Scissor = {};

VkDescriptorSetLayout _descriptorLayout = nullptr;
VkDescriptorPool _descriptorPool = nullptr;
VkDescriptorSet _descriptorSet = nullptr;

VkImage _depthImage = nullptr;
VkImageView _depthView = nullptr;
VkDeviceMemory _depthImageMemory = nullptr;

Graphics::Camera _Camera;

Window::Size _size;

struct Vertex
{
	Core::Vector4f position;
	Core::Vector4f color;
};

constexpr Vertex _triangle[3] = {
	{ { 0.0f, -0.75f, 0.f }, { 1.f, 0.f, 0.f } },
	{ { 0.25f, 0.5f, 0.f }, { 0.f, 1.f, 0.f } },
	{ { -0.25f, 0.5f, 0.f }, { 0.f, 0.f, 1.f } },
};

constexpr Core::Vector4f v0{ -1.f, -1.f, -1.f };
constexpr Core::Vector4f v1{ -1.f, 1.f, -1.f };
constexpr Core::Vector4f v2{ 1.f, 1.f, -1.f };
constexpr Core::Vector4f v3{ 1.f, -1.f, -1.f };

constexpr Core::Vector4f v4{ -1.f, -1.f, 1.f };
constexpr Core::Vector4f v5{ -1.f, 1.f, 1.f };
constexpr Core::Vector4f v6{ 1.f, 1.f, 1.f };
constexpr Core::Vector4f v7{ 1.f, -1.f, 1.f };

constexpr Vertex _cube[] = {
	// front
	{ v2, { 1.f, 0.f, 0.f } }, // 0
	{ v0, { 1.f, 0.f, 0.f } }, // 1
	{ v1, { 1.f, 0.f, 0.f } }, // 2

	{ v2, { 1.f, 0.f, 0.f } }, // 3
	{ v3, { 1.f, 0.f, 0.f } }, // 4
	{ v0, { 1.f, 0.f, 0.f } }, // 5

	// right
	{ v6, { 0.f, 1.f, 0.f } }, // 6
	{ v3, { 0.f, 1.f, 0.f } }, // 7
	{ v2, { 0.f, 1.f, 0.f } }, // 8

	{ v6, { 0.f, 1.f, 0.f } }, // 9
	{ v7, { 0.f, 1.f, 0.f } }, // 10
	{ v3, { 0.f, 1.f, 0.f } }, // 11

	// back
	{ v5, { 0.f, 0.f, 1.f } }, // 12
	{ v7, { 0.f, 0.f, 1.f } }, // 13
	{ v6, { 0.f, 0.f, 1.f } }, // 14

	{ v5, { 0.f, 0.f, 1.f } }, // 15
	{ v4, { 0.f, 0.f, 1.f } }, // 16
	{ v7, { 0.f, 0.f, 1.f } }, // 17

	// left
	{ v1, { 1.f, 1.f, 0.f } }, // 18
	{ v4, { 1.f, 1.f, 0.f } }, // 19
	{ v5, { 1.f, 1.f, 0.f } }, // 20

	{ v1, { 1.f, 1.f, 0.f } }, // 21
	{ v0, { 1.f, 1.f, 0.f } }, // 22
	{ v4, { 1.f, 1.f, 0.f } }, // 23

	// top
	{ v6, { 1.f, 1.f, 1.f } }, // 24
	{ v1, { 1.f, 1.f, 1.f } }, // 25
	{ v5, { 1.f, 1.f, 1.f } }, // 26

	{ v6, { 1.f, 1.f, 1.f } }, // 27
	{ v2, { 1.f, 1.f, 1.f } }, // 28
	{ v1, { 1.f, 1.f, 1.f } }, // 29

	// bottom
	{ v3, { 0.f, 1.f, 1.f } }, // 30
	{ v4, { 0.f, 1.f, 1.f } }, // 31
	{ v0, { 0.f, 1.f, 1.f } }, // 32

	{ v3, { 0.f, 1.f, 1.f } }, // 33
	{ v7, { 0.f, 1.f, 1.f } }, // 34
	{ v4, { 0.f, 1.f, 1.f } }, // 35

};

namespace Graphics
{

	// This should be a memory allocator class

	uint32 findMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags, VkPhysicalDevice device )
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties( device, &memProperties );

		for( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
		{
			if( ( typeFilter & ( 1 << i ) ) &&
				( memProperties.memoryTypes[i].propertyFlags & propertyFlags ) == propertyFlags )
			{
				return i;
			}
		}
		ASSERT( false, "Failed to find suitable memory type!" );
		return 0;
	}

	VkDeviceMemory GPUAllocateMemory( const VkMemoryRequirements& memRequirements, VkDevice logicalDevice,
									  VkPhysicalDevice physDevice )
	{
		VkDeviceMemory memory;
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex =
			findMemoryType( memRequirements.memoryTypeBits,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physDevice );

		if( vkAllocateMemory( logicalDevice, &allocInfo, nullptr, &memory ) != VK_SUCCESS )
			ASSERT( false, "Failed to allocate memory on GPU!" );

		return memory;
	}

	void CreateBuffer( const VkBufferCreateInfo& createInfo, VkBuffer* buffer, VkDeviceMemory* memory, VkDevice device,
					   VkPhysicalDevice physicalDevice )
	{
		if( vkCreateBuffer( device, &createInfo, nullptr, buffer ) != VK_SUCCESS )
			ASSERT( false, "Failed to create vertex buffer!" );

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements( device, *buffer, &memRequirements );

		*memory = GPUAllocateMemory( memRequirements, device, physicalDevice );

		if( vkBindBufferMemory( device, *buffer, *memory, 0 ) != VK_SUCCESS )
			ASSERT( false, "Failed to bind buffer memory!" );
	}
	// to here

	struct GPUBuffer
	{
		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;
	};

	struct VertexBuffer
	{
		GPUBuffer m_VertexBuffer;
		int32 m_VertexCount = 0;
		int32 m_Stride = 0;
		int32 m_Offset = 0;
	};

	struct Cube
	{
		VertexBuffer m_VertexBuffer;
		Core::Matrix44f m_Orientation;

		void Draw( VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout )
		{

			vkCmdPushConstants( commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( Core::Matrix44f ),
								&m_Orientation );
			/* This is quite a strange one, this is only gonna be available in non-instanced entities for position */

			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers( commandBuffer, 0, 1, &m_VertexBuffer.m_VertexBuffer.m_Buffer, &offset );
			vkCmdDraw( commandBuffer, m_VertexBuffer.m_VertexCount, 1, 0, 0 );
		}

		void destroy( VkDevice device ) { vkDestroyBuffer( device, m_VertexBuffer.m_VertexBuffer.m_Buffer, nullptr ); }

		void init( VkDevice device, VkPhysicalDevice physicalDevice )
		{
			m_Orientation = Core::Matrix44f::Identity();
			m_VertexBuffer.m_Stride = sizeof( Vertex );
			m_VertexBuffer.m_VertexCount = ARRSIZE( _cube );
			m_VertexBuffer.m_Offset = 0;

			const int32 dataSize = m_VertexBuffer.m_Stride * m_VertexBuffer.m_VertexCount;

			VkBufferCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			createInfo.size = dataSize;
			createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkDeviceMemory deviceMem;
			VkBuffer buffer;

			CreateBuffer( createInfo, &buffer, &deviceMem, device, physicalDevice );

			m_VertexBuffer.m_VertexBuffer.m_Buffer = buffer;
			m_VertexBuffer.m_VertexBuffer.m_Memory = deviceMem;

			void* data = nullptr;
			if( vkMapMemory( device, deviceMem, 0, dataSize, 0, &data ) != VK_SUCCESS )
				ASSERT( false, "Failed to map memory!" );
			memcpy( data, _cube, dataSize );
			vkUnmapMemory( device, deviceMem );
		}

		void setPosition( const Core::Vector4f& position ) { m_Orientation.SetPosition( position ); }
	};

	struct Shader
	{
		VkShaderModule m_Module = nullptr;

		void Create( VkShaderModule module ) { m_Module = module; }

		VkShaderModule GetModule() { return m_Module; }
	};

	Shader _vertexShader;
	Shader _fragmentShader;

	std::vector<Cube> _Cubes;

	ConstantBuffer _ViewProjection;

	vkGraphicsDevice::vkGraphicsDevice() = default;

	vkGraphicsDevice::~vkGraphicsDevice()
	{

		auto device = m_LogicalDevice->GetDevice();
		DestroyConstantBuffer( &_ViewProjection );

		for( Cube& cube : _Cubes )
			cube.destroy( m_LogicalDevice->GetDevice() );

		DestroyShader( &_vertexShader );
		DestroyShader( &_fragmentShader );

		vkDestroyPipelineLayout( device, _pipelineLayout, nullptr );
		vkDestroyPipeline( device, _pipeline, nullptr );

		vkDestroySemaphore( device, m_DrawDone, nullptr );
		vkDestroySemaphore( device, m_AcquireNextImageSemaphore, nullptr );
		vkDestroyCommandPool( device, m_CmdPool, nullptr );

		for( VkFramebuffer buffer : m_FrameBuffers )
			vkDestroyFramebuffer( device, buffer, nullptr );
	}

	bool vkGraphicsDevice::Init( const Window& window )
	{
		_size = window.GetInnerSize();
		_Camera.InitPerspectiveProjection( _size.m_Width, _size.m_Height, 0.1f, 1000.f, 90.f );
		_Camera.SetTranslation( { 0.f, 0.f, -25.f, 1.f } );

		m_Instance = std::make_unique<VlkInstance>();
		m_Instance->Init();

		m_PhysicalDevice = std::make_unique<VlkPhysicalDevice>();
		m_PhysicalDevice->Init( m_Instance.get() );

		m_LogicalDevice = std::make_unique<VlkDevice>();
		m_LogicalDevice->Init( m_PhysicalDevice.get() );

		m_Swapchain = std::make_unique<VlkSwapchain>();
		m_Swapchain->Init( m_Instance.get(), m_LogicalDevice.get(), m_PhysicalDevice.get(), window );

		_ViewProjection.RegVar( _Camera.GetView() );
		_ViewProjection.RegVar( _Camera.GetProjection() );
		//_ViewProjection.RegVar( _Camera.GetViewProjectionPointer() );

		CreateConstantBuffer( &_ViewProjection );
		CreateCommandPool();
		CreateCommandBuffer();
		CreateDepthResources();
		_renderPass = CreateRenderPass();

		m_FrameBuffers.resize( m_Swapchain->GetNofImages() );
		auto& list = m_Swapchain->GetImageList();
		auto& viewList = m_Swapchain->GetImageViewList();
		for( int i = 0; i < m_FrameBuffers.size(); i++ )
		{
			viewList[i] = CreateImageView( m_Swapchain->GetFormat().format, list[i], VK_IMAGE_ASPECT_COLOR_BIT );

			VkImageView views[] = { viewList[i], _depthView };
			m_FrameBuffers[i] = CreateFramebuffer( views, ARRSIZE( views ), window );
		}

		LoadShader( &_vertexShader, "Data/Shaders/vertex.vert" );
		LoadShader( &_fragmentShader, "Data/Shaders/frag.frag" );

		CreateViewport( 0.f, 0.f, _size.m_Width, _size.m_Height, 0.f, 1.f, &_Viewport );
		SetupScissorArea( (uint32)_size.m_Width, (uint32)_size.m_Height, 0, 0, &_Scissor );

		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.descriptorCount = 1;

		VkDescriptorSetLayoutBinding bindings[] = {
			uboLayoutBinding,
		};

		_descriptorLayout = CreateDescriptorLayout( bindings, ARRSIZE( bindings ) );

		VkDescriptorSetLayout descriptorLayouts[] = {
			_descriptorLayout,
		};

		VkPushConstantRange pushRangeList[] = {
			{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( Core::Matrix44f ) },
		};

		_pipelineLayout = CreatePipelineLayout( descriptorLayouts, ARRSIZE( descriptorLayouts ), pushRangeList,
												ARRSIZE( pushRangeList ) );
		_pipeline = CreateGraphicsPipeline();

		m_AcquireNextImageSemaphore = CreateVkSemaphore( m_LogicalDevice->GetDevice() );
		m_DrawDone = CreateVkSemaphore( m_LogicalDevice->GetDevice() );

		const float xValue = -22.f;
		const float yValue = -12.f;
		const float zValue = 0.f;
		Core::Vector4f position{ xValue, yValue, zValue, 1.f };

		for( int i = 0; i < 128; i++ )
		{
			_Cubes.push_back( Cube() );
			Cube& last = _Cubes.back();
			last.init( m_LogicalDevice->GetDevice(), m_PhysicalDevice->GetDevice() );
			last.setPosition( position );

			position.x += 5.f;
			if( i % 10 == 0 && i != 0 )
			{
				position.y += 5.f;
				position.x = xValue;
			}
		}

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		if( vkCreateFence( m_LogicalDevice->GetDevice(), &fenceCreateInfo, nullptr, &m_CommandFence ) != VK_SUCCESS )
			ASSERT( false, "Failed to create fence!" );

		return true;
	}
	//_____________________________________________

	void vkGraphicsDevice::SetupScissorArea( uint32 width, uint32 height, int32 offsetX, int32 offsetY,
											 VkRect2D* scissorArea )
	{
		scissorArea->extent.width = width;
		scissorArea->extent.height = height;
		scissorArea->offset.x = offsetX;
		scissorArea->offset.y = offsetY;
	}

	//_____________________________________________

	void vkGraphicsDevice::DrawFrame( float dt )
	{
		if( vkAcquireNextImageKHR( m_LogicalDevice->GetDevice(), m_Swapchain->GetSwapchain(), UINT64_MAX,
								   m_AcquireNextImageSemaphore, VK_NULL_HANDLE /*fence*/, &m_Index ) != VK_SUCCESS )
			ASSERT( false, "Failed to acquire next image!" );

		Input::InputManager& input = Input::InputManager::Get();

		Input::HInputDeviceMouse* mouse = nullptr;
		input.GetDevice( Input::EDeviceType_Mouse, &mouse );

		const Input::Cursor& cursor = mouse->GetCursor();

		Input::HInputDeviceKeyboard* keyboard = nullptr;
		input.GetDevice( Input::EDeviceType_Keyboard, &keyboard );
		const float speed = 10.f;

		if( keyboard->IsDown( DIK_W ) )
		{
			_Camera.Forward( speed * dt );
		}

		if( keyboard->IsDown( DIK_S ) )
		{
			_Camera.Forward( -speed * dt );
		}
		if( keyboard->IsDown( DIK_D ) )
		{
			_Camera.Right( speed * dt );
		}
		if( keyboard->IsDown( DIK_A ) )
		{
			_Camera.Right( -speed * dt );
		}
		if( keyboard->IsDown( DIK_R ) )
		{
			_Camera.Up( speed * dt );
		}
		if( keyboard->IsDown( DIK_F ) )
		{
			_Camera.Up( -speed * dt );
		}

		if( mouse->IsDown( 1 ) )
		{
			_Camera.OrientCamera( { cursor.dx, cursor.dy } );
		}

		_Camera.Update();
		BindConstantBuffer( &_ViewProjection, 0 );

		const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // associated with
																									 // having
																									 // semaphores
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitDstStageMask;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CmdBuffers[m_Index];

		submitInfo.pSignalSemaphores = &m_DrawDone;
		submitInfo.signalSemaphoreCount = 1;

		submitInfo.pWaitSemaphores = &m_AcquireNextImageSemaphore;
		submitInfo.waitSemaphoreCount = 1;

		// This line fails when running with renderdoc, suspecting empty queue would be the issue
		if( vkQueueSubmit( m_LogicalDevice->GetQueue(), 1, &submitInfo, m_CommandFence ) != VK_SUCCESS )
			ASSERT( false, "Failed to submit the queue!" );

		VkSwapchainKHR swapchain = m_Swapchain->GetSwapchain();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &m_Index;
		presentInfo.pWaitSemaphores = &m_DrawDone;
		presentInfo.waitSemaphoreCount = 1;

		if( vkQueuePresentKHR( m_LogicalDevice->GetQueue(), &presentInfo ) != VK_SUCCESS )
			ASSERT( false, "Failed to present!" );

		SetupRenderCommands( m_Index ^ 1 );
	}

	//_____________________________________________

	VkRenderPass vkGraphicsDevice::CreateRenderPass()
	{

		VkAttachmentDescription attDesc = {};
		attDesc.format = m_Swapchain->GetFormat().format;
		attDesc.samples = VK_SAMPLE_COUNT_1_BIT;

		attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference attRef = {};
		attRef.attachment = 0;
		attRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDesc = {};
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pColorAttachments = &attRef;
		subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[] = { attDesc, depthAttachment };
		VkRenderPassCreateInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rpInfo.attachmentCount = ARRSIZE( attachments );
		rpInfo.pAttachments = attachments;
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpassDesc;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		VkRenderPass renderpass = nullptr;
		if( vkCreateRenderPass( m_LogicalDevice->GetDevice(), &rpInfo, nullptr, &renderpass ) != VK_SUCCESS )
			ASSERT( false, "Failed to create renderpass" );

		return renderpass;
	}
	//_____________________________________________

	void vkGraphicsDevice::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolCreateInfo.pNext = nullptr;
		poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolCreateInfo.queueFamilyIndex = m_PhysicalDevice->GetQueueFamilyIndex();

		if( vkCreateCommandPool( m_LogicalDevice->GetDevice(), &poolCreateInfo, nullptr, &m_CmdPool ) != VK_SUCCESS )
			ASSERT( false, "failed to create VkCommandPool!" );
	}
	//_____________________________________________

	void vkGraphicsDevice::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
		cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocInfo.commandPool = m_CmdPool;
		cmdBufAllocInfo.commandBufferCount = (uint32)m_Swapchain->GetNofImages();
		cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		m_CmdBuffers.resize( m_Swapchain->GetNofImages() );

		if( vkAllocateCommandBuffers( m_LogicalDevice->GetDevice(), &cmdBufAllocInfo, m_CmdBuffers.data() ) !=
			VK_SUCCESS )
			ASSERT( false, "Failed to create VkCommandBuffer!" );
	}
	//_____________________________________________

	VkPipeline vkGraphicsDevice::CreateGraphicsPipeline()
	{

		// viewport
		VkPipelineViewportStateCreateInfo vpCreateInfo = {};
		vpCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vpCreateInfo.viewportCount = 1;
		vpCreateInfo.scissorCount = 1;
		vpCreateInfo.pScissors = &_Scissor;
		vpCreateInfo.pViewports = &_Viewport;

		// depth
		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
		pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
		pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.minDepthBounds = 0.f;
		pipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.f;
		pipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;

		// rasterizer
		VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
		rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // VK_POLYGON_MODE_LINE // Wireframe
		rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rastCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rastCreateInfo.depthClampEnable = VK_FALSE;
		rastCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rastCreateInfo.depthBiasEnable = VK_FALSE;
		rastCreateInfo.lineWidth = 1.0f;

		// sampler
		VkPipelineMultisampleStateCreateInfo pipelineMSCreateInfo = {};
		pipelineMSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMSCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineMSCreateInfo.sampleShadingEnable = VK_FALSE;

		// blendstate
		VkPipelineColorBlendAttachmentState blendAttachState = {};
		blendAttachState.colorWriteMask = 0xF;
		blendAttachState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
		blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendCreateInfo.logicOpEnable = VK_FALSE;
		blendCreateInfo.attachmentCount = 1;
		blendCreateInfo.pAttachments = &blendAttachState;

		auto bindDesc = CreateBindDesc();
		auto attrDesc = CreateAttrDesc( 0, 0 );
		auto attrDesc2 = CreateAttrDesc( 1, 16 );

		VkVertexInputBindingDescription bindDescArr[] = { bindDesc };
		VkVertexInputAttributeDescription descriptions[] = { attrDesc, attrDesc2 };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = ARRSIZE( bindDescArr );
		vertexInputInfo.vertexAttributeDescriptionCount = ARRSIZE( descriptions );

		vertexInputInfo.pVertexBindingDescriptions = bindDescArr;
		vertexInputInfo.pVertexAttributeDescriptions = descriptions;

		CreateDescriptorPool();
		CreateDescriptorSet();

		VkDescriptorBufferInfo bInfo2 = {};
		bInfo2.buffer = static_cast<VkBuffer>( _ViewProjection.GetBuffer() );
		bInfo2.offset = 0;
		bInfo2.range = _ViewProjection.GetSize();

		VkWriteDescriptorSet descWrite = {};
		descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite.dstSet = _descriptorSet;
		descWrite.dstBinding = 0;
		descWrite.dstArrayElement = 0;
		descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descWrite.descriptorCount = 1;
		descWrite.pBufferInfo = &bInfo2;

		vkUpdateDescriptorSets( m_LogicalDevice->GetDevice(), 1, &descWrite, 0, nullptr );

		VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {};
		pipelineIACreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineIACreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineIACreateInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineShaderStageCreateInfo ssci[] = {
			CreateShaderStageInfo( VK_SHADER_STAGE_VERTEX_BIT, _vertexShader.GetModule(), "main" ),
			CreateShaderStageInfo( VK_SHADER_STAGE_FRAGMENT_BIT, _fragmentShader.GetModule(), "main" )
		};

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = _pipelineLayout;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &pipelineIACreateInfo;
		pipelineInfo.renderPass = _renderPass;
		pipelineInfo.pViewportState = &vpCreateInfo;
		pipelineInfo.pColorBlendState = &blendCreateInfo;
		pipelineInfo.pRasterizationState = &rastCreateInfo;
		pipelineInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
		pipelineInfo.pMultisampleState = &pipelineMSCreateInfo;

		pipelineInfo.pStages = ssci;
		pipelineInfo.stageCount = ARRSIZE( ssci );

		VkPipeline pipeline;
		if( vkCreateGraphicsPipelines( m_LogicalDevice->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
									   &pipeline ) != VK_SUCCESS )
			ASSERT( false, "Failed to create pipeline!" );
		return pipeline;
	}

	VkDescriptorSetLayout vkGraphicsDevice::CreateDescriptorLayout( VkDescriptorSetLayoutBinding* descriptorBindings,
																	int32 bindingCount )
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = bindingCount;
		layoutInfo.pBindings = descriptorBindings;
		VkDescriptorSetLayout descriptorLayout;
		if( vkCreateDescriptorSetLayout( m_LogicalDevice->GetDevice(), &layoutInfo, nullptr, &descriptorLayout ) !=
			VK_SUCCESS )
			ASSERT( false, "Failed to create Descriptor layout" );

		return descriptorLayout;
	}

	void vkGraphicsDevice::CreateDescriptorPool()
	{

		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 1; // (uint32_t)m_Swapchain->GetNofImages();

		VkDescriptorPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = 1;
		createInfo.pPoolSizes = &poolSize;
		createInfo.maxSets = (uint32_t)m_Swapchain->GetNofImages();

		if( vkCreateDescriptorPool( m_LogicalDevice->GetDevice(), &createInfo, nullptr, &_descriptorPool ) !=
			VK_SUCCESS )
			ASSERT( false, "Failed to create descriptorPool" );
	}

	void vkGraphicsDevice::CreateDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = 1; // (uint32_t)m_Swapchain->GetNofImages();

		VkDescriptorSetLayout layouts[] = {
			_descriptorLayout,
		};

		allocInfo.pSetLayouts = layouts;

		if( vkAllocateDescriptorSets( m_LogicalDevice->GetDevice(), &allocInfo, &_descriptorSet ) != VK_SUCCESS )
			ASSERT( false, "failed to allocate descriptor sets!" );
	}

	//_____________________________________________

	VkPipelineLayout vkGraphicsDevice::CreatePipelineLayout( VkDescriptorSetLayout* descriptorLayouts,
															 int32 descriptorLayoutCount,
															 VkPushConstantRange* pushConstantRange,
															 int32 pushConstantRangeCount )
	{

		VkPipelineLayoutCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineCreateInfo.setLayoutCount = descriptorLayoutCount;
		pipelineCreateInfo.pSetLayouts = descriptorLayouts;

		pipelineCreateInfo.pushConstantRangeCount = pushConstantRangeCount;
		pipelineCreateInfo.pPushConstantRanges = pushConstantRange;

		VkPipelineLayout pipeline;
		if( vkCreatePipelineLayout( m_LogicalDevice->GetDevice(), &pipelineCreateInfo, nullptr, &pipeline ) !=
			VK_SUCCESS )
			ASSERT( false, "Failed to create pipelineLayout" );
		return pipeline;
	}
	//_____________________________________________

	VkImageView vkGraphicsDevice::CreateImageView( VkFormat format, VkImage image, VkImageAspectFlags aspectFlag )
	{
		VkImageViewCreateInfo vcInfo = {};
		vcInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vcInfo.image = image;
		vcInfo.format = format;
		vcInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		vcInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
							  VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		VkImageSubresourceRange& srr = vcInfo.subresourceRange;
		srr.aspectMask = aspectFlag;
		srr.baseMipLevel = 0;
		srr.levelCount = 1;
		srr.baseArrayLayer = 0;
		srr.layerCount = 1;
		VkImageView view;
		if( vkCreateImageView( m_LogicalDevice->GetDevice(), &vcInfo, nullptr, &view ) != VK_SUCCESS )
			ASSERT( false, "Failed to create ImageView!" );

		return view;
	}

	VkFramebuffer vkGraphicsDevice::CreateFramebuffer( VkImageView* view, int32 attachmentCount, const Window& window )
	{

		VkFramebufferCreateInfo fbInfo = {};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = _renderPass;
		fbInfo.attachmentCount = attachmentCount;
		fbInfo.pAttachments = view;
		fbInfo.width = (uint32)window.GetInnerSize().m_Width;
		fbInfo.height = (uint32)window.GetInnerSize().m_Height;
		fbInfo.layers = 1;

		VkFramebuffer framebuffer;
		if( vkCreateFramebuffer( m_LogicalDevice->GetDevice(), &fbInfo, nullptr, &framebuffer ) != VK_SUCCESS )
			ASSERT( false, "Failed to create framebuffer!" );
		return framebuffer;
	}

	VkVertexInputBindingDescription vkGraphicsDevice::CreateBindDesc()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof( Vertex );
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	VkVertexInputAttributeDescription vkGraphicsDevice::CreateAttrDesc( int location, int offset )
	{
		VkVertexInputAttributeDescription attrDesc = {};
		attrDesc.binding = 0;
		attrDesc.location = location;
		attrDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attrDesc.offset = offset;
		return attrDesc;
	}

	VkSemaphore vkGraphicsDevice::CreateVkSemaphore( VkDevice pDevice )
	{
		VkSemaphore semaphore = nullptr;
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if( vkCreateSemaphore( pDevice, &semaphoreCreateInfo, nullptr, &semaphore ) != VK_SUCCESS )
			ASSERT( false, "Failed to create VkSemaphore" );

		return semaphore;
	}
	//_____________________________________________

	VkShaderModule vkGraphicsDevice::LoadShader( const char* filepath, VkDevice pDevice )
	{
		Core::File shader( filepath, Core::FileMode::READ_FILE );
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shader.GetSize();
		createInfo.pCode = (const uint32_t*)shader.GetBuffer();

		VkShaderModule shaderModule = nullptr;
		if( vkCreateShaderModule( pDevice, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS )
			ASSERT( false, "Failed to create VkShaderModule!" );

		return shaderModule;
	}

	void vkGraphicsDevice::LoadShader( HShader* shader, const char* filepath )
	{
		shader->Create( LoadShader( filepath, m_LogicalDevice->GetDevice() ) );
	}

	void vkGraphicsDevice::BindConstantBuffer( ConstantBuffer* constantBuffer, uint32 offset )
	{
		auto lDevice = m_LogicalDevice->GetDevice();

		VkDeviceMemory deviceMem = static_cast<VkDeviceMemory>( constantBuffer->GetDeviceMemory() );
		void* data = nullptr;
		if( vkMapMemory( lDevice, deviceMem, offset, constantBuffer->GetSize(), 0, &data ) != VK_SUCCESS )
			ASSERT( false, "Failed to map memory!" );

		int8* pMem = static_cast<int8*>( data );
		int32 step = 0;
		for( auto it : constantBuffer->GetVariables() )
		{
			memcpy( &pMem[step], static_cast<int8*>( it.var ), it.size );
			step += it.size;
		}
		vkUnmapMemory( lDevice, deviceMem );
	}

	void vkGraphicsDevice::DestroyConstantBuffer( ConstantBuffer* constantBuffer )
	{
		vkDestroyBuffer( m_LogicalDevice->GetDevice(), static_cast<VkBuffer>( constantBuffer->GetBuffer() ), nullptr );
	}

	//_____________________________________________

	void vkGraphicsDevice::CreateConstantBuffer( ConstantBuffer* constantBuffer )
	{
		VkBufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = constantBuffer->GetSize();
		createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto lDevice = m_LogicalDevice->GetDevice();
		auto pDevice = m_PhysicalDevice->GetDevice();

		VkBuffer vulkan_buffer = nullptr;
		if( vkCreateBuffer( lDevice, &createInfo, nullptr, &vulkan_buffer ) != VK_SUCCESS )
			ASSERT( false, "Failed to create vertex buffer!" );

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements( lDevice, vulkan_buffer, &memRequirements );

		VkDeviceMemory deviceMem = GPUAllocateMemory( memRequirements, lDevice, pDevice );

		if( vkBindBufferMemory( lDevice, vulkan_buffer, deviceMem, 0 ) != VK_SUCCESS )
			ASSERT( false, "Failed to bind buffer memory!" );

		constantBuffer->SetBuffer( vulkan_buffer );
		constantBuffer->SetDeviceMemory( deviceMem );
	}

	void vkGraphicsDevice::SetupRenderCommands( int index )
	{
		vkWaitForFences( m_LogicalDevice->GetDevice(), 1, &m_CommandFence, VK_TRUE, UINT64_MAX );
		vkResetFences( m_LogicalDevice->GetDevice(), 1, &m_CommandFence );

		VkClearValue clearValue[2] = {};
		clearValue[0].color = _clearColor;
		clearValue[1].depthStencil = { 1.f, 0 };

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _renderPass;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { (uint32)_size.m_Width, (uint32)_size.m_Height };
		renderPassInfo.clearValueCount = ARRSIZE( clearValue );
		renderPassInfo.pClearValues = clearValue;

		VkCommandBufferBeginInfo cmdInfo = {};
		cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		VkFramebuffer& frameBuffer = m_FrameBuffers[index];
		VkCommandBuffer& commandBuffer = m_CmdBuffers[index];

		if( vkBeginCommandBuffer( commandBuffer, &cmdInfo ) != VK_SUCCESS )
			ASSERT( false, "Failed to begin CommandBuffer!" );
		renderPassInfo.framebuffer = frameBuffer;

		vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
		vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
		vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet,
								 0, nullptr );

		for( Cube& cube : _Cubes )
			cube.Draw( commandBuffer, _pipelineLayout );

		vkCmdEndRenderPass( commandBuffer );

		if( vkEndCommandBuffer( commandBuffer ) != VK_SUCCESS )
			ASSERT( false, "Failed to end CommandBuffer!" );
	}

	void vkGraphicsDevice::DestroyShader( HShader* pShader )
	{
		vkDestroyShaderModule( m_LogicalDevice->GetDevice(), pShader->GetModule(), nullptr );
	}

	Camera* vkGraphicsDevice::GetCamera() { return &_Camera; }

	VkFormat vkGraphicsDevice::findSupportedFormat( const std::vector<VkFormat>& candidates, VkImageTiling tiling,
													VkFormatFeatureFlags features )
	{
		for( VkFormat format : candidates )
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties( m_PhysicalDevice->GetDevice(), format, &props );

			if( tiling == VK_IMAGE_TILING_LINEAR && ( props.linearTilingFeatures & features ) == features )
			{
				return format;
			}
			else if( tiling == VK_IMAGE_TILING_OPTIMAL && ( props.optimalTilingFeatures & features ) == features )
			{
				return format;
			}
		}

		ASSERT( false, "undefined format!" );
		return VK_FORMAT_UNDEFINED;
	}

	VkFormat vkGraphicsDevice::findDepthFormat()
	{
		return findSupportedFormat( { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
									VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
	}

	bool hasStencilComponent( VkFormat format )
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void vkGraphicsDevice::CreateDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();
		const VkExtent2D extent = m_Swapchain->GetExtent();
		CreateImage( extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
					 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage,
					 _depthImageMemory );
		_depthView = CreateImageView( depthFormat, _depthImage, VK_IMAGE_ASPECT_DEPTH_BIT );

		transitionImageLayout( _depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
							   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
	}

	uint32_t vkGraphicsDevice::findMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties )
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties( m_PhysicalDevice->GetDevice(), &memProperties );

		for( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
		{
			if( ( typeFilter & ( 1 << i ) ) &&
				( memProperties.memoryTypes[i].propertyFlags & properties ) == properties )
			{
				return i;
			}
		}

		throw std::runtime_error( "failed to find suitable memory type!" );
	}

	void vkGraphicsDevice::CreateImage( uint32 width, uint32 height, VkFormat format, VkImageTiling imageTiling,
										VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
										VkDeviceMemory& imageMemory )
	{
		VkDevice device = m_LogicalDevice->GetDevice();

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = imageTiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if( vkCreateImage( device, &imageInfo, nullptr, &image ) != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to create image!" );
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements( device, image, &memRequirements );

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, properties );

		if( vkAllocateMemory( device, &allocInfo, nullptr, &imageMemory ) != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to allocate image memory!" );
		}

		vkBindImageMemory( device, image, imageMemory, 0 );
	}

	void vkGraphicsDevice::transitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout,
												  VkImageLayout newLayout )
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		if( newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if( hasStencilComponent( format ) )
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
				 newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			throw std::invalid_argument( "unsupported layout transition!" );
		}

		vkCmdPipelineBarrier( commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier );

		vkCmdPipelineBarrier( commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier );

		endSingleTimeCommands( commandBuffer );
	}

	void vkGraphicsDevice::copyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size )
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

		endSingleTimeCommands( commandBuffer );
	}

	VkCommandBuffer vkGraphicsDevice::beginSingleTimeCommands()
	{
		VkDevice device = m_LogicalDevice->GetDevice();
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_CmdPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers( device, &allocInfo, &commandBuffer );

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer( commandBuffer, &beginInfo );

		return commandBuffer;
	}

	void vkGraphicsDevice::endSingleTimeCommands( VkCommandBuffer commandBuffer )
	{
		vkEndCommandBuffer( commandBuffer );

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit( m_LogicalDevice->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE );
		vkQueueWaitIdle( m_LogicalDevice->GetQueue() );

		vkFreeCommandBuffers( m_LogicalDevice->GetDevice(), m_CmdPool, 1, &commandBuffer );
	}

	VkPipelineShaderStageCreateInfo vkGraphicsDevice::CreateShaderStageInfo( VkShaderStageFlagBits stageFlags,
																			 VkShaderModule module,
																			 const char* entryPoint )
	{
		VkPipelineShaderStageCreateInfo stageInfo = {};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = stageFlags;
		stageInfo.module = module;
		stageInfo.pName = entryPoint;
		return stageInfo;
	}

	void vkGraphicsDevice::CreateViewport( float topLeftX, float topLeftY, float width, float height, float minDepth,
										   float maxDepth, VkViewport* viewport )
	{
		viewport->x = topLeftX;
		viewport->y = topLeftY;
		viewport->width = width;
		viewport->height = height;
		viewport->minDepth = minDepth;
		viewport->maxDepth = maxDepth;
	}

}; // namespace Graphics
