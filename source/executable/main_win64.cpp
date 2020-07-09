#include "graphics/Window.h"
#include "graphics/GraphicsEngine.h"

#include "core/Timer.h"
#include "input/InputManager.h"
#include "Logger/Debug.h"

#include "game/StateStack.h"
#include "game/Game.h"
#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_win32.h"

#include <memory>
#include <iostream>
#include <Windows.h>

#include "core/utilities/utilities.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE instance, HINSTANCE /*prevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	Log::Debug::Create();

	Window::CreateInfo createInfo{ 1920.f, 1080.f, instance, WindowProc };

	Window window(createInfo);
	window.SetText("Kaffe b�nan");

	Graphics::GraphicsEngine::Create();
	Graphics::GraphicsEngine& graphics_engine = Graphics::GraphicsEngine::Get();
	Graphics::CreateImGuiContext();
	if(!graphics_engine.Init(window))
		return -1;

	Input::InputManager& input = Input::InputManager::Get();
	input.Initialize(window.GetHandle(), instance);

	Core::Timer timer;
	timer.Init();

	ImGui_ImplWin32_Init(window.GetHandle());

	Game game;
	StateStack state_stack;
	state_stack.PushState(&game, StateStack::MAIN);

	MSG msg;
	do
	{
		timer.Update();

		char temp[100] = { 0 };
		sprintf_s(temp, "FPS : %.3f dt: %.3f", 1.f / timer.GetTime(), timer.GetTime());
		window.SetText(temp);

		/* Windows Specific */
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) // message pump
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(msg.message == WM_QUIT || msg.message == WM_CLOSE)
		{
			break;
		}
		/* Windows Specific */

		state_stack.UpdateCurrentState(timer.GetTime());
		input.Update();
		// graphics_engine.Update();
		graphics_engine.Present(timer.GetTime());

	} while(true);

	Input::InputManager::Destroy();

	Log::Debug::Destroy();

	return 0;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) /* very windows
																					  specific */
{

	if(ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	switch(message)
	{
		case WM_CREATE:
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_SIZE:
			break;
		case WM_ACTIVATE:
		{
			if(LOWORD(wParam) == WA_INACTIVE)
			{
			}
			else
			{
			}
		}
		break;
		case WM_ENTERSIZEMOVE:
			break;
		case WM_EXITSIZEMOVE:
			break;
		case WM_CLOSE:
			break;
		case WM_SYSCOMMAND:
			if((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return false;
			break;
		case WM_SYSKEYDOWN:
			if(wParam == VK_RETURN)
			{
				if((HIWORD(lParam) & KF_ALTDOWN))
				{
				}
			}
			break;
		case WM_INPUT:
		{
		}
		break;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
