#pragma once

#include "GraphicsDevice.h"
#include <memory>

class Window;
class Camera;

namespace Graphics
{
	void CreateImGuiContext();
	class GraphicsEngine
	{
	public:
		GraphicsEngine();
		~GraphicsEngine();

		static void Create();
		static GraphicsEngine& Get();

		bool Init(const Window& window);
		void Present(float dt);

	private:
		static std::unique_ptr<GraphicsEngine> m_Instance;

		void BeginFrame();
	};

}; // namespace Graphics
