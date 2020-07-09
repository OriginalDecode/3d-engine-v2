#include "GraphicsEngine.h"
#include "Window.h"

#include <cassert>

#ifdef _WIN32
#include <d3d11.h>
#include <Windows.h>
#endif

#include "vkGraphicsDevice.h"

#include "imgui/imgui.h"

constexpr float BLACK[4] = { 0.f, 0.f, 0.f, 0.f };

namespace Graphics
{
	void CreateImGuiContext()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
	}

	std::unique_ptr<GraphicsEngine> GraphicsEngine::m_Instance = nullptr;

	GraphicsEngine::GraphicsEngine() {}

	void GraphicsEngine::Create() { m_Instance = std::make_unique<GraphicsEngine>(); }

	GraphicsEngine& GraphicsEngine::Get() { return *m_Instance; }

	bool GraphicsEngine::Init(const Window& window)
	{
		vkGraphicsDevice::Create();
		vkGraphicsDevice& device = vkGraphicsDevice::Get();
		if(!device.Init(window))
			return false;

		return true;
	}

	void GraphicsEngine::Present(float dt) { vkGraphicsDevice::Get().DrawFrame(dt); }

	void GraphicsEngine::BeginFrame() {}

	GraphicsEngine::~GraphicsEngine() { vkGraphicsDevice::Destroy(); }

}; // namespace Graphics
