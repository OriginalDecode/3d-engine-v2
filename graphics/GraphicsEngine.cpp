#include "GraphicsEngine.h"

#include <cassert>
#include <d3d11.h>

#include "Window.h"

#pragma comment(lib, "d3d11.lib")

constexpr float BLACK[4] = { 0.f, 0.f, 0.f, 0.f };

ID3D11Device* device = nullptr;
IDXGISwapChain* swapchain = nullptr;

namespace Graphics
{
	GraphicsEngine::~GraphicsEngine()
	{
		m_DefaultRenderTarget->Release();
		m_DefaultDepthTarget->Release();
		swapchain->Release();
		device->Release();
	}

	ID3D11Device* GetDevice()
	{
		return device;
	}

	void GraphicsEngine::CreateDeviceAndSwapchain(const Window& window)
	{
		DXGI_SWAP_CHAIN_DESC scDesc;
		ZeroMemory(&scDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		const Window::Size& window_size = window.GetInnerSize();

		scDesc.BufferCount = 1;
		scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scDesc.BufferDesc.Width = UINT(window_size.m_Width);
		scDesc.BufferDesc.Height = UINT(window_size.m_Height);
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.OutputWindow = window.GetHandle();
		scDesc.SampleDesc.Count = 1;
		scDesc.SampleDesc.Quality = 0;
		scDesc.Windowed = true;

		scDesc.Flags = 0;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		const D3D_FEATURE_LEVEL requested_feature_levels[] = {
			D3D_FEATURE_LEVEL_11_0,
		};
		const UINT feature_count = ARRAYSIZE(requested_feature_levels);

		UINT create_device_flags = 0;
#ifdef _DEBUG
		create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr,
												   D3D_DRIVER_TYPE_HARDWARE,
												   nullptr,
												   create_device_flags,
												   requested_feature_levels,
												   feature_count,
												   D3D11_SDK_VERSION,
												   &scDesc,
												   &swapchain,
												   &device,
												   nullptr,
												   nullptr);

		assert(!FAILED(hr) && "Failed to create Device and Swapchain");


		ID3D11DeviceContext* context = nullptr;
		device->GetImmediateContext(&context);

		ID3D11Texture2D* backbuffer = nullptr;

		hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
		assert(!FAILED(hr) && "Failed to create backbuffer");

		hr = swapchain->SetFullscreenState(FALSE, nullptr);
		assert(!FAILED(hr) && "Failed to set to fullscreen");

		hr = device->CreateRenderTargetView(backbuffer, nullptr, &m_DefaultRenderTarget);
		assert(!FAILED(hr) && "Failed to create RenderTargetView");
		backbuffer->Release();

		D3D11_TEXTURE2D_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(depthDesc));

		depthDesc.Width = UINT(window_size.m_Width);
		depthDesc.Height = UINT(window_size.m_Height);
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D32_FLOAT; //DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = 1; //sample
		depthDesc.SampleDesc.Quality = 0; //quality pattern
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D* depth_buffer = nullptr;
		hr = device->CreateTexture2D(&depthDesc, nullptr, &depth_buffer);
		assert(!FAILED(hr) && "Failed to create texture for depthbuffer");

		D3D11_DEPTH_STENCIL_VIEW_DESC stencilDesc;
		ZeroMemory(&stencilDesc, sizeof(stencilDesc));
		stencilDesc.Format = depthDesc.Format;
		stencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		stencilDesc.Texture2D.MipSlice = 0;

		hr = device->CreateDepthStencilView(depth_buffer, &stencilDesc, &m_DefaultDepthTarget);
		assert(!FAILED(hr) && "Failed to create depthstencil view");
		depth_buffer->Release();

		context->OMSetRenderTargets(1, &m_DefaultRenderTarget, m_DefaultDepthTarget);


		context->Release();
	}

	void GraphicsEngine::Present()
	{
		HRESULT hr = swapchain->Present(1, 0);
		switch (hr)
		{
			case DXGI_ERROR_DEVICE_HUNG:
				OutputDebugString("device hung\n");
				break;

			case DXGI_ERROR_DEVICE_REMOVED:
				OutputDebugString("removed\n");
				break;

			case DXGI_ERROR_DEVICE_RESET:
				OutputDebugString("reset\n");
				break;

			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				OutputDebugString("internal_error\n");
				break;

			case DXGI_ERROR_INVALID_CALL:
				OutputDebugString("invalid_call\n");
				break;
		};

		BeginFrame();

	}

	void GraphicsEngine::BeginFrame()
	{
		ID3D11DeviceContext* context = nullptr;
		device->GetImmediateContext(&context);
		context->ClearRenderTargetView(m_DefaultRenderTarget, BLACK);
		context->ClearDepthStencilView(m_DefaultDepthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		context->Release();
	}


}; // namespace Graphics