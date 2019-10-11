#include "VulkanDevice.h"

#include "logger/Debug.h"

#include "graphics/GraphicsEngine.h"
#include "graphics/Window.h"

#include <vulkan/vulkan.h>
#ifdef _WIN32
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback( VkDebugReportFlagsEXT /* flags */,
													VkDebugReportObjectTypeEXT /* objectType */, uint64_t /* object */,
													size_t /* location */, int32_t /* messageCode */,
													const char* /* pLayerPrefix */, const char* pMessage,
													void* /* pUserData */ )
{
	char temp[USHRT_MAX] = { 0 };
	sprintf_s( temp, "Vulkan Warning :%s", pMessage );
	OutputDebugStringA( pMessage );
	OutputDebugStringA( "\n" );
	LOG_MESSAGE( temp );
	// assert( false && temp );
	return VK_FALSE;
}

#define VK_GET_FNC_POINTER( function, instance ) ( PFN_##function ) vkGetInstanceProcAddr( instance, #function )

namespace Graphics
{
	VkDebugReportCallbackEXT debugCallback = nullptr;

	VulkanDevice::~VulkanDevice()
	{
		auto destoryer = VK_GET_FNC_POINTER( vkDestroyDebugReportCallbackEXT, m_Instance );
		destoryer( m_Instance, debugCallback, nullptr );

		vkDestroyInstance( m_Instance, nullptr );
		vkDestroyDevice( m_Device, nullptr );
	}

	// void VulkanDevice::GetSwapchainImages( VkSwapchainKHR* pSwapchain, std::vector<VkImage>* scImages )
	//{
	//	uint32 imageCount = 0;
	//	VkResult result = vkGetSwapchainImagesKHR( m_Device, *pSwapchain, &imageCount, nullptr );
	//	assert( result == VK_SUCCESS && "failed to get imagecount" );
	//	scImages->resize( imageCount );
	//	result = vkGetSwapchainImagesKHR( m_Device, *pSwapchain, &imageCount, scImages->data() );
	//	assert( result == VK_SUCCESS && "failed to get swapchainimages" );
	//}

	void VulkanDevice::Init()
	{
		CreateInstance();
		CreatePhysicalDevice();
		CreateDevice();
		CreateSwapchainSurface();
		CreateSwapchain();
	}

	void VulkanDevice::CreateInstance()
	{
		VkApplicationInfo appInfo = {};

		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "wce engine";
		appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.pEngineName = "wce engine";
		appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;

		const char* validationLayers[] = { "VK_LAYER_LUNARG_standard_validation" };
		instanceCreateInfo.enabledLayerCount = ARRSIZE( validationLayers );
		instanceCreateInfo.ppEnabledLayerNames = &validationLayers[0];

		const char* extentions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
		instanceCreateInfo.enabledExtensionCount = ARRSIZE( extentions );
		instanceCreateInfo.ppEnabledExtensionNames = extentions;

		if( vkCreateInstance( &instanceCreateInfo, nullptr /*allocator*/, &m_Instance ) != VK_SUCCESS )
		{
			ASSERT( false, "Failed to create Vulkan instance!" );
		}

		auto create = VK_GET_FNC_POINTER( vkCreateDebugReportCallbackEXT, m_Instance );
		ASSERT( create, "Failed to create callback function!" );

		VkDebugReportFlagsEXT flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
									  VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.pfnCallback = &DebugReportCallback;
		createInfo.flags = flags;

		if( create( m_Instance, &createInfo, nullptr, &debugCallback ) != VK_SUCCESS )
			ASSERT( false, "Failed to create vulkan debug callback!" );
	}

	void VulkanDevice::CreatePhysicalDevice()
	{
		uint32 device_count = 0;
		if( vkEnumeratePhysicalDevices( m_Instance, &device_count, nullptr ) != VK_SUCCESS )
		{
			ASSERT( false, "Failed to enumerate gpus!" );
		}

		VkPhysicalDevice* gpus = new VkPhysicalDevice[device_count];

		if( vkEnumeratePhysicalDevices( m_Instance, &device_count, gpus ) != VK_SUCCESS )
		{
			ASSERT( false, "Failed to enumerate device!" );
		}

		int32 queueIndex = 0;
		int32 familyIndex = 0;
		VkQueueFamilyProperties* properties = nullptr;
		for( int32 i = 0; i < device_count; ++i )
		{
			VkPhysicalDevice device = gpus[i];
			uint32 property_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties( device, &property_count, nullptr );
			properties = new VkQueueFamilyProperties[property_count];
			vkGetPhysicalDeviceQueueFamilyProperties( device, &property_count, properties );

			for( int32 j = 0; j < property_count; ++j )
			{
				VkQueueFamilyProperties property = properties[j];

				if( property.queueCount > 0 && property.queueFlags & VK_QUEUE_GRAPHICS_BIT )
				{
					queueIndex = j;
					break;
				}
			}

			if( queueIndex > -1 )
			{
				m_PhysicalDevice = device;
				m_QueueFamily = queueIndex;
				break;
			}
		}

		delete[] properties;
		delete[] gpus;

		ASSERT( m_PhysicalDevice, "No physical device after creation!" );
	}

	void VulkanDevice::CreateDevice()
	{
		// queue create info
		const float queue_priorities[] = { 1.f };
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = m_QueueFamily;
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
		const char* debugLayers[] = { "VK_LAYER_LUNARG_standard_validation" };
		createInfo.enabledLayerCount = ARRSIZE( debugLayers );
		createInfo.ppEnabledLayerNames = debugLayers;
#endif

		const char* deviceExt[] = { "VK_KHR_swapchain" };
		createInfo.enabledExtensionCount = ARRSIZE( deviceExt );
		createInfo.ppEnabledExtensionNames = deviceExt;
		createInfo.pEnabledFeatures = &enabled_features;

		if( vkCreateDevice( m_PhysicalDevice, &createInfo, nullptr, &m_Device ) != VK_SUCCESS )
		{
			ASSERT( false, "Failed to create device!" );
		}

		ASSERT( m_Device, "Failed to create device!" );

		vkGetDeviceQueue( m_Device, m_QueueFamily, 0 /*queueIndex*/, &m_Queue );
	}

	void VulkanDevice::CreateSwapchain()
	{
		

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = capabilities.minImageCount;
		createInfo.imageFormat = format.format;
		createInfo.imageColorSpace = format.colorSpace;
		createInfo.imageExtent = vkExtent;
		createInfo.imageArrayLayers = 1;

		if( queueProp.queueIndex != queueProp.familyIndex )
		{
			const uint32_t queueIndices[] = { (uint32_t)queueProp.queueIndex, (uint32_t)queueProp.familyIndex };
			createInfo.queueFamilyIndexCount = ARRSIZE( queueIndices );
			createInfo.pQueueFamilyIndices = queueIndices;
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		}
		else
		{
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		VkSurfaceTransformFlagBitsKHR transformFlags = capabilities.currentTransform;
		if( capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
			transformFlags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

		createInfo.preTransform = transformFlags;

		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // this field defines blocking or nonblocking -
																// (VK_PRESENT_MODE_FIFO_KHR) blocks (vsync on
																// or off)

		createInfo.clipped = VK_TRUE;

		if( vkCreateSwapchainKHR( m_Device, &createInfo, nullptr, &m_Swapchain ) != VK_SUCCESS )
		{
			ASSERT( false, "failed to create vkSwapchainKHR" );
		}
	}

	void VulkanDevice::CreateSwapchainSurface() 
	{
		const Window* window = GraphicsEngine::Get().GetWindow();
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = static_cast<HWND>( window->GetHandle() );
		createInfo.hinstance = ::GetModuleHandle( nullptr );
		 
		if (vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) != VK_SUCCESS)
		{
			ASSERT( false, "Failed to create Win32SurfaceKHR" );
		}
	}

}; // namespace Graphics