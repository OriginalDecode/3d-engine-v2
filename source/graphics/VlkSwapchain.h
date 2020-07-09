#pragma once

#include "Core/Defines.h"
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vector>
DEFINE_HANDLE(VkSwapchainKHR);
DEFINE_HANDLE(VkSurfaceKHR);

// typedef void HImage;

struct VkSurfaceCapabilitiesKHR;
struct VkSurfaceFormatKHR;

class Window;
class VlkDevice;
class VlkPhysicalDevice;
class VlkInstance;
class VlkSurface;

class VlkSwapchain
{
public:
	VkSurfaceFormatKHR GetFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkSurfaceFormatKHR VlkSwapchain::GetFormat();
	VlkSwapchain();
	~VlkSwapchain();
	void Release();

	void Init(VlkInstance* instance, VlkDevice* device, VlkPhysicalDevice* physicalDevice, const Window& window);
	size_t GetNofImages() const { return m_Images.size(); }

	std::vector<VkImage>& GetImageList() { return m_Images; }
	std::vector<VkImageView>& GetImageViewList() { return m_ImageViews; }
	VkSwapchainKHR GetSwapchain() { return m_Swapchain; }
	VkExtent2D GetExtent() const;

	VlkSurface* GetSurface();

private:
	std::unique_ptr<VlkSurface> m_Surface;
	VkSwapchainKHR m_Swapchain;
	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;

	struct
	{
		struct
		{
			float x = 0.f;
			float y = 0.f;
		} offset;

		struct
		{
			float x = 0.f;
			float y = 0.f;
		} width;

	} m_Extent;
};
