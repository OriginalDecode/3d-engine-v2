#pragma once

#ifdef LINUX
#include <X11/Xlib.h>
#endif
#include "Core/Types.h"

#ifdef _WIN32
#ifndef _WINDEF_
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
struct HWND__;
typedef HWND__* HWND;
#endif
typedef HWND HWindow;
#else
typedef void* HWindow; //X11 Window
#endif

class Window
{
public:
	struct Size
	{
		Size() = default;
		Size(const float width, const float height) : m_Width(width), m_Height(height) { }
		float m_Width = 0;
		float m_Height = 0;
	};

	struct CreateInfo
	{
		CreateInfo() = default;
		CreateInfo(const float width, const float height)
			: m_Size(Size(width, height))
		{
		}

#ifdef _WIN32
		CreateInfo(const float width, const float height, HINSTANCE instance_handle, void* window_proc)
			: m_Size(Size(width, height))
			, m_InstanceHandle(instance_handle)
			, m_WindowProcess(window_proc)
		{
		}
		HINSTANCE m_InstanceHandle = nullptr;
		void* m_WindowProcess;
#endif
		Size m_Size;
	};

	Window( const CreateInfo& info );

	Window() = default;
	~Window();


	void ShowWindow();

	const Size& GetSize() const { return m_WindowSize; }
	const Size& GetInnerSize() const { return m_InnerSize; }

	void SetText(const char* window_text); 

	
	HWindow GetHandle() const { return m_WindowHandle; }
private:
	HWindow m_WindowHandle;

	bool m_WindowIsActive = false;
	bool m_IsFullScreen = false;
	
	Size m_WindowSize;
	Size m_InnerSize;

};

