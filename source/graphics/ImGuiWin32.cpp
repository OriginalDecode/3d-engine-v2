#include "ImGuiWin32.h"

#include "imgui/imgui.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <xinput.h>
#include <tchar.h>

ImGuiMouseCursor g_LastMouseCursor = ImGuiMouseCursor_COUNT;

bool ImGuiWin32::Init(void* init_data)
{

	// this can be replaced with my own timer class
	if(!::QueryPerformanceFrequency((LARGE_INTEGER*)&m_TicksPerSecond))
		return false;
	if(!::QueryPerformanceCounter((LARGE_INTEGER*)&m_Time))
		return false;
	// gege
	

	ImGuiWin32_InitData* data = (ImGuiWin32_InitData*)init_data;
	m_hWnd = data->m_hWnd;

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.BackendPlatformName = "Win32";
	io.ImeWindowHandle = m_hWnd;

	MapKeyboard(&io);

	return true;
}

void ImGuiWin32::MapKeyboard(ImGuiIO* io)
{
	// very win platform exclusive
	io->KeyMap[ImGuiKey_Tab] = VK_TAB;
	io->KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io->KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io->KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io->KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io->KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io->KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io->KeyMap[ImGuiKey_Home] = VK_HOME;
	io->KeyMap[ImGuiKey_End] = VK_END;
	io->KeyMap[ImGuiKey_Insert] = VK_INSERT;
	io->KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io->KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io->KeyMap[ImGuiKey_Space] = VK_SPACE;
	io->KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io->KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io->KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
	io->KeyMap[ImGuiKey_A] = 'A';
	io->KeyMap[ImGuiKey_C] = 'C';
	io->KeyMap[ImGuiKey_V] = 'V';
	io->KeyMap[ImGuiKey_X] = 'X';
	io->KeyMap[ImGuiKey_Y] = 'Y';
	io->KeyMap[ImGuiKey_Z] = 'Z';
}

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef DBT_DEVNODES_CHANGED
constexpr uint32 DBT_DEVNODES_CHANGED = 0x0007;
#endif

int64_ptr ImGuiWin32::WndProcHandler(HWND hwnd, uint32 msg, uint64_ptr wParam, int64_ptr lParam)
{

	if(ImGui::GetCurrentContext() == NULL)
		return 0;

	ImGuiIO& io = ImGui::GetIO();
	switch(msg)
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		{
			int button = 0;
			if(msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK)
			{
				button = 0;
			}
			if(msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK)
			{
				button = 1;
			}
			if(msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK)
			{
				button = 2;
			}
			if(msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK)
			{
				button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
			}
			if(!ImGui::IsAnyMouseDown() && ::GetCapture() == NULL)
				::SetCapture(hwnd);
			io.MouseDown[button] = true;
			return 0;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			int button = 0;
			if(msg == WM_LBUTTONUP)
			{
				button = 0;
			}
			if(msg == WM_RBUTTONUP)
			{
				button = 1;
			}
			if(msg == WM_MBUTTONUP)
			{
				button = 2;
			}
			if(msg == WM_XBUTTONUP)
			{
				button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
			}
			io.MouseDown[button] = false;
			if(!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd)
				::ReleaseCapture();
			return 0;
		}
		case WM_MOUSEWHEEL:
			io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
			return 0;
		case WM_MOUSEHWHEEL:
			io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
			return 0;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if(wParam < 256)
				io.KeysDown[wParam] = 1;
			return 0;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if(wParam < 256)
				io.KeysDown[wParam] = 0;
			return 0;
		case WM_CHAR:
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			io.AddInputCharacter((unsigned int)wParam);
			return 0;
		case WM_SETCURSOR:
			if(LOWORD(lParam) == HTCLIENT /*&& ImGui_ImplWin32_UpdateMouseCursor()*/)
				return 1;
			return 0;
		case WM_DEVICECHANGE:
			if((UINT)wParam == DBT_DEVNODES_CHANGED)
				// g_WantUpdateHasGamepad = true;
				return 0;
	}
	return 0;
}

void ImGuiWin32::NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt() &&
			  "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

	// Setup display size (every frame to accommodate for window resizing)
	RECT rect;
	::GetClientRect(m_hWnd, &rect);
	io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

	// Setup time step
	INT64 current_time;
	::QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
	io.DeltaTime = (float)(current_time - m_Time) / m_TicksPerSecond;
	m_Time = current_time;

	// Read keyboard modifiers inputs
	io.KeyCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;

	// Update OS mouse position
	// ImGui_ImplWin32_UpdateMousePos();

	// Update OS mouse cursor with the cursor requested by imgui
	ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
	if(g_LastMouseCursor != mouse_cursor)
	{
		g_LastMouseCursor = mouse_cursor;
		// ImGui_ImplWin32_UpdateMouseCursor();
	}

	// Update game controllers (if enabled and available)
	// ImGui_ImplWin32_UpdateGamepads();
}
