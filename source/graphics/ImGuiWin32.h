#pragma once
#include "ImGuiBase.h"
#include "core/Defines.h"
#include "core/Types.h"

struct ImGuiWin32_InitData
{
	HWND m_hWnd;
};

struct ImGuiIO;
class ImGuiWin32 final : public ImGuiBase
{
public:
	virtual bool Init(void* init_data) override;
	virtual void NewFrame() override;

	int64_ptr WndProcHandler(HWND hwnd, uint32 msg, uint64_ptr wParam, int64_ptr lParam);

private:
	HWND m_hWnd;
	int64 m_Time;
	int64 m_TicksPerSecond;
	bool m_HasGamepad = false;
	bool m_WantUpdateHasGamepad = false;

	void MapKeyboard(ImGuiIO* io);
};
