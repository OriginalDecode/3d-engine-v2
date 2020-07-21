#pragma once

class ImGuiBase
{
public:
	ImGuiBase() = default;
	virtual ~ImGuiBase() = 0;

	virtual bool Init(void* init_data) = 0;
	virtual void NewFrame() = 0;

private:
};
