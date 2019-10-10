#pragma once
#include <Core/Defines.h>
#include "graphics/IGfxDevice.h"
#include <vulkan/vulkan_core.h>
#include <vector>

LPE_DEFINE_HANDLE( VkDevice );
LPE_DEFINE_HANDLE( VkQueue );
LPE_DEFINE_HANDLE( VkSwapchainKHR );
LPE_DEFINE_HANDLE( VkSurfaceKHR );

struct VkSwapchainCreateInfoKHR;
struct VkWin32SurfaceCreateInfoKHR;

namespace Graphics
{
	class VlkPhysicalDevice;
	class VlkDevice : public IGfxDevice
	{
	public:
		VlkDevice() = default;
		~VlkDevice();

		void Init( VlkPhysicalDevice* physicalDevice );

		VkDevice GetDevice() const { return m_Device; }
		VkQueue GetQueue() const { return m_Queue; }

		VkSwapchainKHR CreateSwapchain( const VkSwapchainCreateInfoKHR& createInfo ) const;
		void DestroySwapchain( VkSwapchainKHR pSwapchain );

		void GetSwapchainImages( VkSwapchainKHR* pSwapchain, std::vector<VkImage>* scImages );

	private:
		void Release( IGfxDevice* device ) override;
		VkDevice m_Device = nullptr;
		VkQueue m_Queue = nullptr;
	};

}; //namespace Graphics