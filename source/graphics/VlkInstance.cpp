#include "VlkInstance.h"
#include "VlkSurface.h"
#include "VlkPhysicalDevice.h"
#include "logger/Debug.h"

#include <vulkan/vulkan.h>
#ifdef _WIN32
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#include <vector>
#include <cstdio>
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT /* flags */,
												   VkDebugReportObjectTypeEXT /* objectType */, uint64_t /* object */,
												   size_t /* location */, int32_t /* messageCode */,
												   const char* /* pLayerPrefix */, const char* pMessage,
												   void* /* pUserData */)
{
	char temp[USHRT_MAX] = { 0 };
	snprintf(temp, USHRT_MAX, "Vulkan Warning :%s", pMessage);
	LOG_MESSAGE(temp);
	return VK_FALSE;
}

#define VK_GET_FNC_POINTER(function, instance) (PFN_##function) vkGetInstanceProcAddr(instance, #function)

constexpr char* validationLayers[] = { "VK_LAYER_LUNARG_standard_validation" };
constexpr char* extentions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
VkDebugReportCallbackEXT debugCallback = nullptr;

void SetupDebugCallback(VkInstance instance)
{
	auto FCreateCallback = VK_GET_FNC_POINTER(vkCreateDebugReportCallbackEXT, instance);
	ASSERT(FCreateCallback, "Failed to setup callback!");

	if(!FCreateCallback)
		return;

	VkDebugReportFlagsEXT flags =
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.pfnCallback = &DebugReportCallback;
	createInfo.flags = flags;

	VERIFY(FCreateCallback(instance, &createInfo, nullptr, &debugCallback) == VK_SUCCESS, "Failed to create callback");
}

VlkInstance::~VlkInstance()
{
	Release();
}

void VlkInstance::Init()
{
	VkApplicationInfo appInfo = {};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Kaffe Bönan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Kaffe Bönan";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo instanceCreateInfo = {};

	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	instanceCreateInfo.enabledLayerCount = ARRSIZE(validationLayers);
	instanceCreateInfo.ppEnabledLayerNames = &validationLayers[0];

	instanceCreateInfo.enabledExtensionCount = ARRSIZE(extentions);
	instanceCreateInfo.ppEnabledExtensionNames = extentions;

	VERIFY(vkCreateInstance(&instanceCreateInfo, nullptr /*allocator*/, &m_Instance) == VK_SUCCESS, "Failed to create instance");

	SetupDebugCallback(m_Instance);
}

void VlkInstance::Release()
{
	auto destoryer = VK_GET_FNC_POINTER(vkDestroyDebugReportCallbackEXT, m_Instance);
	destoryer(m_Instance, debugCallback, nullptr);

	vkDestroyInstance(m_Instance, nullptr);
}

VkSurfaceKHR VlkInstance::CreateSurface(const VkWin32SurfaceCreateInfoKHR& createInfo) const
{
	VkSurfaceKHR surface = nullptr;
#ifdef _WIN32
	VERIFY(vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &surface) == VK_SUCCESS, "Failed to create surface");
#endif
	return surface;
}

std::unique_ptr<VlkSurface> VlkInstance::CreateSurface(const VkWin32SurfaceCreateInfoKHR& createInfo,
													   VlkPhysicalDevice* physicalDevice) const
{
	auto surface = std::make_unique<VlkSurface>();
	surface->Init(CreateSurface(createInfo), physicalDevice);
	return surface;
}

void VlkInstance::DestroySurface(VkSurfaceKHR pSurface)
{
	vkDestroySurfaceKHR(m_Instance, pSurface, nullptr);
}

void VlkInstance::GetPhysicalDevices(std::vector<VkPhysicalDevice>& deviceList)
{
	uint32 device_count = 0;
	VERIFY(vkEnumeratePhysicalDevices(m_Instance, &device_count, nullptr) == VK_SUCCESS, "Failed to enum device!");

	deviceList.resize(device_count);
	VERIFY(vkEnumeratePhysicalDevices(m_Instance, &device_count, deviceList.data()) == VK_SUCCESS, "Failed to enum devices!");
}
