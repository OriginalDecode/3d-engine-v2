#include "VlkSurface.h"

#include "VlkInstance.h"
#include "vkGraphicsDevice.h"
#include "VlkPhysicalDevice.h"
#include "GraphicsEngine.h"

#include <cassert>

VlkSurface::VlkSurface() = default;
VlkSurface::~VlkSurface()
{
	Release();
}

void VlkSurface::Release()
{
	vkGraphicsDevice::Get().GetVlkInstance().DestroySurface(m_Surface);
}

void VlkSurface::Init(VkSurfaceKHR pSurface, VlkPhysicalDevice* physicalDevice)
{
	m_Surface = pSurface;
	physicalDevice->GetSurfaceInfo(pSurface, &m_CanPresent, &m_FormatCount, &m_Formats, &m_PresentCount,
								   &m_PresentModes, &m_Capabilities);
}

bool VlkSurface::CanPresent() const
{
	return m_CanPresent;
}

uint32 VlkSurface::GetPresentModeCount(VkPhysicalDevice pPhysicalDevice) const
{
	uint32 out = 0;
	VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevice, m_Surface, &out, nullptr);
	assert(result == VK_SUCCESS);
	return out;
}

uint32 VlkSurface::GetFormatCount() const
{

	return m_FormatCount;
}

const std::vector<VkSurfaceFormatKHR>& VlkSurface::GetSurfaceFormats() const
{
	return m_Formats;
}

const VkSurfaceCapabilitiesKHR& VlkSurface::GetCapabilities() const
{
	return m_Capabilities;
}
