#pragma once

class ImGuiWrapper 
{
public:
    ImGuiWrapper() = default;
    ~ImGuiWrapper() = default;
    
    bool NewFrame(/* parameters */);
    void EndFrame(/* parameters */);
    
    void NewTable(/* parameters */);
    
private:
};